/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 Systems and Networking, University of Augsburg, Germany
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Ciprian Radu <radu@informatik.uni-augsburg.de>
 */

#include "ns3/log.h"
#include "noc-router.h"
#include "ns3/noc-node.h"
#include "ns3/integer.h"
#include "ns3/noc-registry.h"

NS_LOG_COMPONENT_DEFINE ("NocRouter");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocRouter);

  NocRouter::NocRouter (std::string name)
  {
    m_name = name;
    m_powerCounter = 0;
    m_lastClock = 0;
    m_arrivedFlits = 0;
    m_loadComponent = 0;
    NS_LOG_DEBUG ("No load router component is used");
  }

  NocRouter::NocRouter (std::string name, Ptr<LoadRouterComponent> loadComponent)
  {
    NS_ASSERT_MSG (loadComponent != 0, "The load router component must be specified!"
        " If you do not want to use a load router component, use another constructor.");
    m_name = name;
    m_powerCounter = 0;
    m_lastClock = 0;
    m_arrivedFlits = 0;
    m_loadComponent = loadComponent;
    NS_LOG_DEBUG ("Using the load router component " << loadComponent->GetName ());
  }

  TypeId
  NocRouter::GetTypeId ()
  {
    static TypeId tid = TypeId("ns3::NocRouter")
        .SetParent<Object> ();
    return tid;
  }

  NocRouter::~NocRouter ()
  {
    m_nocNode = 0;
    m_routingProtocol = 0;
  }

  Ptr<Route>
  NocRouter::ManageFlit (const Ptr<NocNetDevice> source, const Ptr<NocNode> destination, Ptr<Packet> flit)
  {
    NS_LOG_FUNCTION_NOARGS();
    NS_ASSERT (source != 0);
    NS_ASSERT (destination != 0);

    uint32_t sourceNodeId = source->GetNode ()->GetId ();
    uint32_t destinationNodeId = destination->GetId ();
    Ptr<Route> route = 0;
    if (sourceNodeId == destinationNodeId)
      {
        NS_LOG_WARN ("Trying to route a packet from node " << sourceNodeId
            << " to node " << destinationNodeId << " (same node)");
      }
    else
      {
        m_powerCounter++;
        TimeValue timeValue;
        NocRegistry::GetInstance ()->GetAttribute ("GlobalClock", timeValue);
        Time globalClock = timeValue.Get ();
        uint64_t clockNumber = Simulator::Now ().GetPicoSeconds () / globalClock.GetPicoSeconds () + 1;
        NS_LOG_DEBUG ("clock number " << clockNumber);
        NS_LOG_DEBUG ("last clock " << m_lastClock);
        NS_LOG_DEBUG ("This router belongs to NoC node " << GetNocNode ()->GetId ());
        if (clockNumber > m_lastClock)
          {
            // the following loop measures the power consumed by the flits from the previous clock cycle (m_lastClock)
            for (unsigned int i = 0; i < m_flitsFromLastClock.size (); ++i)
              {
                MeasurePowerAndEnergy (m_flitsFromLastClock[i]);
              }

            m_flitsFromLastClock.clear ();
            m_arrivedFlits = 1;
            NS_LOG_DEBUG ("# arrived flits reset to " << m_arrivedFlits);
            m_lastClock = clockNumber;
          }
        else
          {
            m_arrivedFlits++;
            NS_LOG_DEBUG ("# arrived flits set to " << m_arrivedFlits);
          }
        m_flitsFromLastClock.insert (m_flitsFromLastClock.end (), flit);

        route = GetRoutingProtocol()->RequestRoute (source, destination, flit);
      }
    return route;
  }

  Ptr<LoadRouterComponent>
  NocRouter::GetLoadRouterComponent () const
  {
    return m_loadComponent;
  }

  void
  NocRouter::AddNeighborLoad (int load, Ptr<NocNetDevice> sourceDevice)
  {
    ; // This method is meant to be overridden if you want it to do something
  }

  int
  NocRouter::GetNeighborLoad (Ptr<NocNetDevice> sourceDevice)
  {
    // This method is meant to be overridden by the subclassing routers which work with load information
    return 0;
  }

  int
  NocRouter::GetNeighborLoad (Ptr<NocNetDevice> sourceDevice, int direction, int dimension)
  {
    // This method is meant to be overridden by the subclassing routers which work with load information
    return 0;
  }

  double
  NocRouter::GetInChannelsOccupancy (Ptr<NocNetDevice> sourceDevice)
  {
    double occupancy = 0;
    uint32_t packets = 0;
    uint64_t sizes = 0;

    for (uint32_t i = 0; i < GetNDevices (); ++i)
      {
        Ptr<NocNetDevice> device = GetDevice (i);
        packets += device->GetInQueueNPacktes ();
        sizes += device->GetInQueueSize ();
      }
    if (sizes != 0 && packets != 0)
      {
        occupancy = packets * 1.0 / sizes;
      }
    NS_LOG_LOGIC ("In channels occupancy is " << occupancy);

    return occupancy;
  }

  double
  NocRouter::GetOutChannelsOccupancy (Ptr<NocNetDevice> sourceDevice)
  {
    double occupancy = 0;
    uint32_t packets = 0;
    uint64_t sizes = 0;

    for (uint32_t i = 0; i < GetNDevices (); ++i)
      {
        Ptr<NocNetDevice> device = GetDevice (i);
        packets += device->GetOutQueueNPacktes ();
        sizes += device->GetOutQueueSize ();
      }
    if (sizes != 0 && packets != 0)
      {
        occupancy = packets * 1.0 / sizes;
      }
    NS_LOG_LOGIC ("Out channels occupancy is " << occupancy);

    return occupancy;
  }

  void
  NocRouter::SetRoutingProtocol (Ptr<NocRoutingProtocol> routingProtocol)
  {
    std::ostringstream oss;
    oss << "Setting a '" << routingProtocol->GetName ()
        << "' routing protocol for the router of the NoC node " << m_nocNode->GetId ();
    NS_LOG_DEBUG (oss.str());

    m_routingProtocol = routingProtocol;
  }

  Ptr<NocRoutingProtocol>
  NocRouter::GetRoutingProtocol ()
  {
    if (m_routingProtocol == 0)
      {
        std::ostringstream oss;
        oss << "No routing protocol is defined for the NoC node " << m_nocNode-> GetId ();
        NS_LOG_WARN(oss.str());
      }
    return m_routingProtocol;
  }

  void
  NocRouter::SetSwitchingProtocol (Ptr<NocSwitchingProtocol> switchingProtocol)
  {
    std::ostringstream oss;
    oss << "Setting a '" << switchingProtocol->GetName ()
        << "' switching protocol for the router of the NoC node " << m_nocNode->GetId ();
    NS_LOG_DEBUG (oss.str());

    m_switchingProtocol = switchingProtocol;
  }

  Ptr<NocSwitchingProtocol>
  NocRouter::GetSwitchingProtocol ()
  {
    if (m_switchingProtocol == 0)
      {
        std::ostringstream oss;
        oss << "No switching protocol is defined for the NoC node " << m_nocNode-> GetId ();
        NS_LOG_WARN(oss.str());
      }
    return m_switchingProtocol;
  }

  uint32_t
  NocRouter::AddDevice (Ptr<NocNetDevice> device)
  {
    uint32_t index = m_devices.size ();
    m_devices.push_back (device);
    return index;
  }

  Ptr<NocNetDevice>
  NocRouter::GetDevice (uint32_t index) const
  {
    NS_ASSERT_MSG (index < m_devices.size (), "Device index " << index <<
                   " is out of range (only have " << m_devices.size () << " devices).");
    return m_devices[index];
  }

  uint32_t
  NocRouter::GetNDevices () const
  {
    return m_devices.size ();
  }

  void
  NocRouter::SetNocNode(Ptr<NocNode> nocNode)
  {
    m_nocNode = nocNode;
  }

  Ptr<NocNode>
  NocRouter::GetNocNode() const
  {
    return m_nocNode;
  }

  std::string
  NocRouter::GetName() const
  {
   return m_name;
  }

  int
  NocRouter::RouterInitForOrion (SIM_router_info_t *info, SIM_router_power_t *router_power, SIM_router_area_t *router_area)
  {
    u_int line_width;
    int share_buf, outdrv;

    /* PHASE 1: set parameters */
    /* general parameters */
    info->n_in = GetNumberOfInputPorts ();
    info->n_cache_in = PARM(cache_in_port);
    info->n_mc_in = PARM(mc_in_port);
    info->n_io_in = PARM(io_in_port);
    info->n_total_in = GetNumberOfInputPorts () + PARM(cache_in_port) + PARM(mc_in_port) + PARM(io_in_port);
    info->n_out = GetNumberOfOutputPorts ();
    info->n_cache_out = PARM(cache_out_port);
    info->n_mc_out = PARM(mc_out_port);
    info->n_io_out = PARM(io_out_port);
    info->n_total_out = GetNumberOfOutputPorts () + PARM(cache_out_port) + PARM(mc_out_port) + PARM(io_out_port);
    IntegerValue flitSize;
    NocRegistry::GetInstance ()->GetAttribute ("FlitSize", flitSize);
    info->flit_width = flitSize.Get ();

    /* virtual channel parameters */
    info->n_v_channel = MAX(GetNumberOfVirtualChannels (), 1);
    info->n_v_class = MAX(PARM(v_class), 1);
    info->cache_class = MAX(PARM(cache_class), 1);
    info->mc_class = MAX(PARM(mc_class), 1);
    info->io_class = MAX(PARM(io_class), 1);
    /* shared buffer implies buffer has tags */
    /* separate buffer & shared switch implies buffer has tri-state output driver*/
    if (info->n_v_class * info->n_v_channel > 1) {
            info->in_share_buf = PARM(in_share_buf);
            info->out_share_buf = PARM(out_share_buf);
            info->in_share_switch = PARM(in_share_switch);
            info->out_share_switch = PARM(out_share_switch);
    }
    else {
            info->in_share_buf = 0;
            info->out_share_buf = 0;
            info->in_share_switch = 0;
            info->out_share_switch = 0;
    }

    /* crossbar */
    info->crossbar_model = PARM(crossbar_model);
    info->degree = PARM(crsbar_degree);
    info->connect_type = PARM(connect_type);
    info->trans_type = PARM(trans_type);
    info->xb_in_seg = PARM(xb_in_seg);
    info->xb_out_seg = PARM(xb_out_seg);
    info->crossbar_in_len = PARM(crossbar_in_len);
    info->crossbar_out_len = PARM(crossbar_out_len);
    /* HACK HACK HACK */
    info->exp_xb_model = PARM(exp_xb_model);
    info->exp_in_seg = PARM(exp_in_seg);
    info->exp_out_seg = PARM(exp_out_seg);

    /* input buffer */
    info->in_buf = 1;
    info->in_buffer_model = PARM(in_buffer_type);
    if(info->in_buf){
            outdrv = !info->in_share_buf && info->in_share_switch;

            uint64_t sizeOfInputBuffers = 0;
            for (uint32_t i = 0; i < GetNDevices (); ++i)
              {
                sizeOfInputBuffers += GetDevice (i)->GetInQueueSize ();
              }
            NS_LOG_DEBUG ("The input buffers of this router have a total size of " << sizeOfInputBuffers << " flits");

            SIM_array_init(&info->in_buf_info, 1, PARM(in_buf_rport), 1, sizeOfInputBuffers, flitSize.Get (), outdrv, info->in_buffer_model);
    }

    if (PARM(cache_in_port)){
    info->cache_in_buf = PARM(cache_in_buf);

            if (info->cache_in_buf){
                    if (PARM(cache_class) > 1){
                    share_buf = info->in_share_buf;
                    outdrv = !share_buf && info->in_share_switch;
                    }
                    else{
                    outdrv = share_buf = 0;
                    }
            SIM_array_init(&info->cache_in_buf_info, 1, PARM(cache_in_buf_rport), 1, PARM(cache_in_buf_set), flitSize.Get (), outdrv, SRAM);
            }
    }

    if (PARM(mc_in_port)){
    info->mc_in_buf = PARM(mc_in_buf);

            if (info->mc_in_buf){
                    if (PARM(mc_class) > 1){
                    share_buf = info->in_share_buf;
                    outdrv = !share_buf && info->in_share_switch;
                    }
                    else{
                    outdrv = share_buf = 0;
                    }
            SIM_array_init(&info->mc_in_buf_info, 1, PARM(mc_in_buf_rport), 1, PARM(mc_in_buf_set), flitSize.Get (), outdrv, SRAM);
            }
    }

    if (PARM(io_in_port)){
    info->io_in_buf = PARM(io_in_buf);

            if (info->io_in_buf){
                    if (PARM(io_class) > 1){
                    share_buf = info->in_share_buf;
                    outdrv = !share_buf && info->in_share_switch;
                    }
                    else{
                    outdrv = share_buf = 0;
                    }
            SIM_array_init(&info->io_in_buf_info, 1, PARM(io_in_buf_rport), 1, PARM(io_in_buf_set), flitSize.Get (), outdrv, SRAM);
            }
    }

    /* output buffer */
    info->out_buf = 0;
    info->out_buffer_model = PARM(out_buffer_type);
    if (info->out_buf){
            uint64_t sizeOfOutputBuffers = 0;
            for (uint32_t i = 0; i < GetNDevices (); ++i)
              {
                sizeOfOutputBuffers += GetDevice (i)->GetOutQueueSize ();
              }
            NS_LOG_DEBUG ("The output buffers of this router have a total size of " << sizeOfOutputBuffers << " flits");

            /* output buffer has no tri-state buffer anyway */
            SIM_array_init(&info->out_buf_info, 1, 1, PARM(out_buf_wport), sizeOfOutputBuffers, flitSize.Get (), 0, info->out_buffer_model);
    }

    /* central buffer */
    info->central_buf = PARM(central_buf);
    if (info->central_buf){
            info->pipe_depth = PARM(pipe_depth);
            /* central buffer is no FIFO */
            SIM_array_init(&info->central_buf_info, 0, PARM(cbuf_rport), PARM(cbuf_wport), PARM(cbuf_set), PARM(cbuf_width) * flitSize.Get (), 0, SRAM);
            /* dirty hack */
            info->cbuf_ff_model = NEG_DFF;
    }

    /* switch allocator input port arbiter */
    if (info->n_v_class * info->n_v_channel > 1) {
            info->sw_in_arb_model = PARM(sw_in_arb_model);
            if (info->sw_in_arb_model) {
                    if (PARM(sw_in_arb_model) == QUEUE_ARBITER) {
                            SIM_array_init(&info->sw_in_arb_queue_info, 1, 1, 1, info->n_v_class*info->n_v_channel, SIM_logtwo(info->n_v_class*info->n_v_channel), 0, REGISTER);
                            if (info->cache_class > 1)
                                    SIM_array_init(&info->cache_in_arb_queue_info, 1, 1, 1, info->cache_class, SIM_logtwo(info->cache_class), 0, REGISTER);
                            if (info->mc_class > 1)
                                    SIM_array_init(&info->mc_in_arb_queue_info, 1, 1, 1, info->mc_class, SIM_logtwo(info->mc_class), 0, REGISTER);
                            if (info->io_class > 1)
                                    SIM_array_init(&info->io_in_arb_queue_info, 1, 1, 1, info->io_class, SIM_logtwo(info->io_class), 0, REGISTER);

                            info->sw_in_arb_ff_model = SIM_NO_MODEL;
                    }
                    else
                            info->sw_in_arb_ff_model = PARM(sw_in_arb_ff_model);
            }
            else
                    info->sw_in_arb_ff_model = SIM_NO_MODEL;
    }
    else {
            info->sw_in_arb_model = SIM_NO_MODEL;
            info->sw_in_arb_ff_model = SIM_NO_MODEL;
    }

    /* switch allocator output port arbiter */
    if(info->n_total_in > 2){
            info->sw_out_arb_model = PARM(sw_out_arb_model);
            if (info->sw_out_arb_model) {
                    if (info->sw_out_arb_model == QUEUE_ARBITER) {
                            line_width = SIM_logtwo(info->n_total_in - 1);
                            SIM_array_init(&info->sw_out_arb_queue_info, 1, 1, 1, info->n_total_in - 1, line_width, 0, REGISTER);
                            info->sw_out_arb_ff_model = SIM_NO_MODEL;
                    }
                    else{
                            info->sw_out_arb_ff_model = PARM(sw_out_arb_ff_model);
                    }
            }
            else{
                    info->sw_out_arb_ff_model = SIM_NO_MODEL;
            }
    }
    else{
            info->sw_out_arb_model = SIM_NO_MODEL;
            info->sw_out_arb_ff_model = SIM_NO_MODEL;
    }

    /* virtual channel allocator type */
    if (info->n_v_channel > 1) {
            info->vc_allocator_type = PARM(vc_allocator_type);
    }
    else
            info->vc_allocator_type = SIM_NO_MODEL;

    /* virtual channel allocator input port arbiter */
    if ( info->n_v_channel > 1 && info->n_in > 1) {
            info->vc_in_arb_model = PARM(vc_in_arb_model);
            if (info->vc_in_arb_model) {
                    if (PARM(vc_in_arb_model) == QUEUE_ARBITER) {
                            SIM_array_init(&info->vc_in_arb_queue_info, 1, 1, 1, info->n_v_channel, SIM_logtwo(info->n_v_channel), 0, REGISTER);
                            info->vc_in_arb_ff_model = SIM_NO_MODEL;
                    }
                    else{
                            info->vc_in_arb_ff_model = PARM(vc_in_arb_ff_model);
                    }
            }
            else {
                    info->vc_in_arb_ff_model = SIM_NO_MODEL;
            }
    }
    else {
            info->vc_in_arb_model = SIM_NO_MODEL;
            info->vc_in_arb_ff_model = SIM_NO_MODEL;
    }

    /* virtual channel allocator output port arbiter */
    if(info->n_in > 1 && info->n_v_channel > 1){
            info->vc_out_arb_model = PARM(vc_out_arb_model);
            if (info->vc_out_arb_model) {
                    if (info->vc_out_arb_model == QUEUE_ARBITER) {
                            line_width = SIM_logtwo((info->n_total_in - 1)*info->n_v_channel);
                            SIM_array_init(&info->vc_out_arb_queue_info, 1, 1, 1, (info->n_total_in -1) * info->n_v_channel, line_width, 0, REGISTER);
                            info->vc_out_arb_ff_model = SIM_NO_MODEL;
                    }
                    else{
                            info->vc_out_arb_ff_model = PARM(vc_out_arb_ff_model);
                    }
            }
            else{
                    info->vc_out_arb_ff_model = SIM_NO_MODEL;
            }
    }
    else{
            info->vc_out_arb_model = SIM_NO_MODEL;
            info->vc_out_arb_ff_model = SIM_NO_MODEL;
    }

    /*virtual channel allocation vc selection model */
    info->vc_select_buf_type = PARM(vc_select_buf_type);
    if(info->vc_allocator_type == VC_SELECT && info->n_v_channel > 1 && info->n_in > 1){
            info->vc_select_buf_type = PARM(vc_select_buf_type);
            SIM_array_init(&info->vc_select_buf_info, 1, 1, 1, info->n_v_channel, SIM_logtwo(info->n_v_channel), 0, info->vc_select_buf_type);
    }
    else{
            info->vc_select_buf_type = SIM_NO_MODEL;
    }


    /* redundant fields */
    if (info->in_buf) {
            if (info->in_share_buf)
                    info->in_n_switch = info->in_buf_info.read_ports;
            else if (info->in_share_switch)
                    info->in_n_switch = 1;
            else
                    info->in_n_switch = info->n_v_class * info->n_v_channel;
    }
    else
            info->in_n_switch = 1;

    if (info->cache_in_buf) {
            if (info->in_share_buf)
                    info->cache_n_switch = info->cache_in_buf_info.read_ports;
            else if (info->in_share_switch)
                    info->cache_n_switch = 1;
            else
                    info->cache_n_switch = info->cache_class;
    }
    else
            info->cache_n_switch = 1;

    if (info->mc_in_buf) {
            if (info->in_share_buf)
                    info->mc_n_switch = info->mc_in_buf_info.read_ports;
            else if (info->in_share_switch)
                    info->mc_n_switch = 1;
            else
                    info->mc_n_switch = info->mc_class;
    }
    else
            info->mc_n_switch = 1;

    if (info->io_in_buf) {
            if (info->in_share_buf)
                    info->io_n_switch = info->io_in_buf_info.read_ports;
            else if (info->in_share_switch)
                    info->io_n_switch = 1;
            else
                    info->io_n_switch = info->io_class;
    }
    else
            info->io_n_switch = 1;

    info->n_switch_in = info->n_in * info->in_n_switch + info->n_cache_in * info->cache_n_switch +
            info->n_mc_in * info->mc_n_switch + info->n_io_in * info->io_n_switch;

    /* no buffering for local output ports */
    info->n_switch_out = info->n_cache_out + info->n_mc_out + info->n_io_out;
    if (info->out_buf) {
            if (info->out_share_buf)
                    info->n_switch_out += info->n_out * info->out_buf_info.write_ports;
            else if (info->out_share_switch)
                    info->n_switch_out += info->n_out;
            else
                    info->n_switch_out += info->n_out * info->n_v_class * info->n_v_channel;
    }
    else
            info->n_switch_out += info->n_out;

    /* clock related parameters */
info->pipeline_stages = PARM(pipeline_stages);
info->H_tree_clock = PARM(H_tree_clock);
info->router_diagonal = PARM(router_diagonal);

    /* PHASE 2: initialization */
    if(router_power){
            SIM_router_power_init(info, router_power);
    }

    if(router_area){
            SIM_router_area_init(info, router_area);
    }

    return 0;
  }

  double
  NocRouter::ComputeRouterEnergyAndPowerWithOrion (SIM_router_info_t *info, SIM_router_power_t *router, int print_depth, char *path, int max_avg, double e_fin, int plot_flag, double freq)
  {
          double Eavg = 0, Eatomic, Estruct, Estatic = 0;
          double Pbuf = 0, Pxbar = 0, Pvc_arbiter = 0, Psw_arbiter = 0, Pclock = 0, Ptotal = 0;
          double Pbuf_static = 0, Pxbar_static = 0, Pvc_arbiter_static = 0, Psw_arbiter_static = 0, Pclock_static = 0;
          double Pbuf_dyn = 0, Pxbar_dyn = 0, Pvc_arbiter_dyn = 0, Psw_arbiter_dyn = 0, Pclock_dyn = 0;
          double e_in_buf_rw, e_cache_in_buf_rw, e_mc_in_buf_rw, e_io_in_buf_rw;
          double e_cbuf_fin, e_cbuf_rw, e_out_buf_rw;
          int next_depth;
          u_int path_len, n_regs;
          int vc_allocator_enabled = 1;

          /* expected value computation */
          e_in_buf_rw       = e_fin * info->n_in;
          e_cache_in_buf_rw = e_fin * info->n_cache_in;
          e_mc_in_buf_rw    = e_fin * info->n_mc_in;
          e_io_in_buf_rw    = e_fin * info->n_io_in;
          e_cbuf_fin        = e_fin * info->n_total_in;
          e_out_buf_rw      = e_cbuf_fin / info->n_total_out * info->n_out;
          e_cbuf_rw         = e_cbuf_fin * info->flit_width / info->central_buf_info.blk_bits;

          next_depth = NEXT_DEPTH(print_depth);
          path_len = SIM_strlen(path);

          /* input buffers */
          if (info->in_buf) {
                  Eavg += SIM_array_stat_energy(&info->in_buf_info, &router->in_buf, e_in_buf_rw, e_in_buf_rw, next_depth, SIM_strcat(path, "input buffer"), max_avg);
                  SIM_res_path(path, path_len);
          }
          if (info->cache_in_buf) {
                  Eavg += SIM_array_stat_energy(&info->cache_in_buf_info, &router->cache_in_buf, e_cache_in_buf_rw, e_cache_in_buf_rw, next_depth, SIM_strcat(path, "cache input buffer"), max_avg);
                  SIM_res_path(path, path_len);
          }
          if (info->mc_in_buf) {
                  Eavg += SIM_array_stat_energy(&info->mc_in_buf_info, &router->mc_in_buf, e_mc_in_buf_rw, e_mc_in_buf_rw, next_depth, SIM_strcat(path, "memory controller input buffer"), max_avg);
                  SIM_res_path(path, path_len);
          }
          if (info->io_in_buf) {
                  Eavg += SIM_array_stat_energy(&info->io_in_buf_info, &router->io_in_buf, e_io_in_buf_rw, e_io_in_buf_rw, next_depth, SIM_strcat(path, "I/O input buffer"), max_avg);
                  SIM_res_path(path, path_len);
          }

          /* output buffers */
          if (info->out_buf) {
                  /* local output ports don't use router buffers */
                  Eavg += SIM_array_stat_energy(&info->out_buf_info, &router->out_buf, e_out_buf_rw, e_out_buf_rw, next_depth, SIM_strcat(path, "output buffer"), max_avg);
                  SIM_res_path(path, path_len);
          }

          /* central buffer */
          if (info->central_buf) {
                  Eavg += SIM_array_stat_energy(&info->central_buf_info, &router->central_buf, e_cbuf_rw, e_cbuf_rw, next_depth, SIM_strcat(path, "central buffer"), max_avg);
                  SIM_res_path(path, path_len);

                  Eavg += SIM_crossbar_stat_energy(&router->in_cbuf_crsbar, next_depth, SIM_strcat(path, "central buffer input crossbar"), max_avg, e_cbuf_fin);
                  SIM_res_path(path, path_len);

                  Eavg += SIM_crossbar_stat_energy(&router->out_cbuf_crsbar, next_depth, SIM_strcat(path, "central buffer output crossbar"), max_avg, e_cbuf_fin);
                  SIM_res_path(path, path_len);

                  /* dirty hack, REMEMBER to REMOVE Estruct and Eatomic */
                  Estruct = 0;
                  n_regs = info->central_buf_info.n_set * (info->central_buf_info.read_ports + info->central_buf_info.write_ports);

                  /* ignore e_switch for now because we overestimate wordline driver cap */

                  Eatomic = router->cbuf_ff.e_keep_0 * (info->pipe_depth - 1) * (n_regs - 2 * (e_cbuf_rw + e_cbuf_rw));
                  SIM_print_stat_energy(SIM_strcat(path, "central buffer pipeline registers/keep 0"), Eatomic, NEXT_DEPTH(next_depth));
                  SIM_res_path(path, path_len);
                  Estruct += Eatomic;

                  Eatomic = router->cbuf_ff.e_clock * (info->pipe_depth - 1) * n_regs;
                  SIM_print_stat_energy(SIM_strcat(path, "central buffer pipeline registers/clock"), Eatomic, NEXT_DEPTH(next_depth));
                  SIM_res_path(path, path_len);
                  Estruct += Eatomic;

                  SIM_print_stat_energy(SIM_strcat(path, "central buffer pipeline registers"), Estruct, next_depth);
                  SIM_res_path(path, path_len);
                  Eavg += Estruct;
          }

          Pbuf_dyn = Eavg * freq;
          Pbuf_static = router->I_buf_static * Vdd * SCALE_S;
          Pbuf = Pbuf_dyn + Pbuf_static;

          /* main crossbar */
          if (info->crossbar_model) {
                  Eavg += SIM_crossbar_stat_energy(&router->crossbar, next_depth, SIM_strcat(path, "crossbar"), max_avg, e_cbuf_fin);
                  SIM_res_path(path, path_len);
          }

          Pxbar_dyn = (Eavg * freq - Pbuf_dyn);
          Pxbar_static = router->I_crossbar_static * Vdd * SCALE_S;
          Pxbar = Pxbar_dyn + Pxbar_static;

          /* switch allocation (arbiter energy only) */
          /* input (local) arbiter for switch allocation*/
          if (info->sw_in_arb_model) {
                  /* assume # of active input arbiters is (info->in_n_switch * info->n_in * e_fin)
                   * assume (info->n_v_channel*info->n_v_class)/2 vcs are making request at each arbiter */

                  Eavg += SIM_arbiter_stat_energy(&router->sw_in_arb, &info->sw_in_arb_queue_info, (info->n_v_channel*info->n_v_class)/2, next_depth, SIM_strcat(path, "switch allocator input arbiter"), max_avg) * info->in_n_switch * info->n_in * e_fin;
                  SIM_res_path(path, path_len);

                  if (info->n_cache_in) {
                          Eavg += SIM_arbiter_stat_energy(&router->cache_in_arb, &info->cache_in_arb_queue_info, e_fin / info->cache_n_switch, next_depth, SIM_strcat(path, "cache input arbiter"), max_avg) * info->cache_n_switch * info->n_cache_in;
                          SIM_res_path(path, path_len);
                  }

                  if (info->n_mc_in) {
                          Eavg += SIM_arbiter_stat_energy(&router->mc_in_arb, &info->mc_in_arb_queue_info, e_fin / info->mc_n_switch, next_depth, SIM_strcat(path, "memory controller input arbiter"), max_avg) * info->mc_n_switch * info->n_mc_in;
                          SIM_res_path(path, path_len);
                  }

                  if (info->n_io_in) {
                          Eavg += SIM_arbiter_stat_energy(&router->io_in_arb, &info->io_in_arb_queue_info, e_fin / info->io_n_switch, next_depth, SIM_strcat(path, "I/O input arbiter"), max_avg) * info->io_n_switch * info->n_io_in;
                          SIM_res_path(path, path_len);
                  }
          }

          /* output (global) arbiter for switch allocation*/
          if (info->sw_out_arb_model) {
                  /* assume # of active output arbiters is (info->n_switch_out * (e_cbuf_fin/info->n_switch_out))
                   * assume (info->n_in)/2 request at each output arbiter */

                  Eavg += SIM_arbiter_stat_energy(&router->sw_out_arb, &info->sw_out_arb_queue_info, info->n_in / 2, next_depth, SIM_strcat(path, "switch allocator output arbiter"), max_avg) * info->n_switch_out * (e_cbuf_fin / info->n_switch_out);

                  SIM_res_path(path, path_len);
          }

          if(info->sw_out_arb_model || info->sw_out_arb_model){
                  Psw_arbiter_dyn = Eavg * freq - Pbuf_dyn - Pxbar_dyn;
                  Psw_arbiter_static = router->I_sw_arbiter_static * Vdd * SCALE_S;
                  Psw_arbiter = Psw_arbiter_dyn + Psw_arbiter_static;
          }

          /* virtual channel allocation (arbiter energy only) */
          /* HACKs:
           *   - assume 1 header flit in every 5 flits for now, hence * 0.2  */

          if(info->vc_allocator_type == ONE_STAGE_ARB && info->vc_out_arb_model  ){
                  /* one stage arbitration (vc allocation)*/
                  /* # of active arbiters */
                  double nActiveArbs = e_fin * info->n_in * 0.2 / 2; //flit_rate * n_in * 0.2 / 2

                  /* assume for each active arbiter, there is 2 requests on average (should use expected value from simulation) */
                  Eavg += SIM_arbiter_stat_energy(&router->vc_out_arb, &info->vc_out_arb_queue_info,
                                  1, next_depth,
                                  SIM_strcat(path, "vc allocation arbiter"),
                                  max_avg) * nActiveArbs;

                  SIM_res_path(path, path_len);
          }
          else if(info->vc_allocator_type == TWO_STAGE_ARB && info->vc_in_arb_model && info->vc_out_arb_model){
                  /* first stage arbitration (vc allocation)*/
                  if (info->vc_in_arb_model) {
                          // # of active stage-1 arbiters (# of new header flits)
                          double nActiveArbs = e_fin * info->n_in * 0.2;


                          /* assume an active arbiter has n_v_channel/2 requests on average (should use expected value from simulation) */
                          Eavg += SIM_arbiter_stat_energy(&router->vc_in_arb, &info->vc_in_arb_queue_info, info->n_v_channel/2, next_depth,
                                          SIM_strcat(path, "vc allocation arbiter (stage 1)"),
                                          max_avg) * nActiveArbs;

                          SIM_res_path(path, path_len);
                  }

                  /* second stage arbitration (vc allocation)*/
                  if (info->vc_out_arb_model) {
                          /* # of active stage-2 arbiters */
                          double nActiveArbs = e_fin * info->n_in * 0.2 / 2; //flit_rate * n_in * 0.2 / 2

                          /* assume for each active arbiter, there is 2 requests on average (should use expected value from simulation) */
                          Eavg += SIM_arbiter_stat_energy(&router->vc_out_arb, &info->vc_out_arb_queue_info,
                                          2, next_depth,
                                          SIM_strcat(path, "vc allocation arbiter (stage 2)"),
                                          max_avg) * nActiveArbs;

                          SIM_res_path(path, path_len);
                  }
          }
          else if(info->vc_allocator_type == VC_SELECT && info->n_v_channel > 1 && info->n_in > 1){
                  double n_read = e_fin * info->n_in * 0.2;
                  double n_write = e_fin * info->n_in * 0.2;
                  Eavg += SIM_array_stat_energy(&info->vc_select_buf_info, &router->vc_select_buf, n_read , n_write, next_depth, SIM_strcat(path, "vc selection"), max_avg);
                  SIM_res_path(path, path_len);

          }
          else{
                  vc_allocator_enabled = 0; //set to 0 means no vc allocator is used
          }

          if(info->n_v_channel > 1 && vc_allocator_enabled){
                  Pvc_arbiter_dyn = Eavg * freq - Pbuf_dyn - Pxbar_dyn - Psw_arbiter_dyn;
                  Pvc_arbiter_static = router->I_vc_arbiter_static * Vdd * SCALE_S;
                  Pvc_arbiter = Pvc_arbiter_dyn + Pvc_arbiter_static;
          }

          /*router clock power (supported for 90nm and below) */
          if(PARM(TECH_POINT) <=90){
                  Eavg += SIM_total_clockEnergy(info, router);
                  Pclock_dyn = Eavg * freq - Pbuf_dyn - Pxbar_dyn - Pvc_arbiter_dyn - Psw_arbiter_dyn;
                  Pclock_static = router->I_clock_static * Vdd * SCALE_S;
                  Pclock = Pclock_dyn + Pclock_static;
          }

          /* static power */
  //      Estatic = router->I_static * Vdd * Period * SCALE_S;

          TimeValue timeValue;
          NocRegistry::GetInstance ()->GetAttribute ("GlobalClock", timeValue);
          Time globalClock = timeValue.Get ();
          double period = globalClock.GetSeconds ();
          Estatic = router->I_static * Vdd * period * SCALE_S;

          SIM_print_stat_energy(SIM_strcat(path, "static energy"), Estatic, next_depth);
          SIM_res_path(path, path_len);
          Eavg += Estatic;
          Ptotal = Eavg * freq;

          SIM_print_stat_energy(path, Eavg, print_depth);

          m_leakagePower += Estatic * freq;
          m_dynamicPower += (Eavg - Estatic) * freq;

          if (plot_flag)
            {
              NS_LOG_DEBUG ("Buffer dynamic power: " << Pbuf);
              NS_LOG_DEBUG ("Crossbar dynamic power: " << Pxbar);
              NS_LOG_DEBUG ("VC_allocator dynamic power: " << Pvc_arbiter);
              NS_LOG_DEBUG ("SW_allocator dynamic power: " << Psw_arbiter);
              NS_LOG_DEBUG ("Clock dynamic power: " << Pclock);

              NS_LOG_LOGIC ("Router static power (consumed so far): " << m_leakagePower);
              NS_LOG_LOGIC ("Router dynamic power (consumed so far): " << m_dynamicPower);
              NS_LOG_LOGIC ("Total router power (static + dynamic, consumed so far): " << Ptotal);
            }

          return Eavg;
  }

  void
  NocRouter::MeasurePowerAndEnergy (Ptr<Packet> flit)
  {
    NS_LOG_FUNCTION (*flit);

    TimeValue timeValue;
    NocRegistry::GetInstance ()->GetAttribute ("GlobalClock", timeValue);
    Time globalClock = timeValue.Get ();

    double freq = 1 / globalClock.GetSeconds ();
    NS_LOG_DEBUG ("NoC clock frequency is " << freq << " Hz");
    double dataWidth = flit->GetSize () * 8; // in bits
    NS_LOG_DEBUG ("Arrived flit has size " << dataWidth);
    NS_LOG_DEBUG ("arrived flits " << m_arrivedFlits);
    NS_LOG_DEBUG ("# input ports " << GetNumberOfInputPorts ());
    double load = m_arrivedFlits * 1.0 / GetNumberOfInputPorts ();
    NS_LOG_DEBUG ("Router load is " << load);
    NS_ASSERT_MSG (load >= 0 && load <= 1, "Router load in [0,1] interval");
    char routerName[] = "NoC router";

    //SIM_router_init(&GLOB(router_info), &GLOB(router_power), NULL);
    RouterInitForOrion(&GLOB(router_info), &GLOB(router_power), NULL);
    //SIM_router_stat_energy(&GLOB(router_info), &GLOB(router_power), print_depth, name, max_flag, load, plot_flag, PARM(Freq));
    ComputeRouterEnergyAndPowerWithOrion (&GLOB(router_info), &GLOB(router_power), 0, routerName, 0, load, 1, freq);

  }

  void
  NocRouter::MeasurePowerAndEnergyForLastClock ()
  {
    NS_LOG_FUNCTION_NOARGS ();

    TimeValue timeValue;
    NocRegistry::GetInstance ()->GetAttribute ("GlobalClock", timeValue);
    Time globalClock = timeValue.Get ();
    uint64_t clockNumber = Simulator::Now ().GetPicoSeconds () / globalClock.GetPicoSeconds () + 1;
    // the following loop measures the power consumed by the flits from the previous clock cycle (m_lastClock)
    for (unsigned int i = 0; i < m_flitsFromLastClock.size (); ++i)
      {
        MeasurePowerAndEnergy (m_flitsFromLastClock[i]);
      }

    m_flitsFromLastClock.clear ();
    m_arrivedFlits = 0;
    NS_LOG_DEBUG ("# arrived flits reset to " << m_arrivedFlits);
    m_lastClock = clockNumber;
  }

  double
  NocRouter::GetDynamicPower ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    MeasurePowerAndEnergyForLastClock ();
    double power = 0;
    NS_LOG_DEBUG ("Power was measured for " << m_powerCounter << " flits");
    if (m_powerCounter > 0)
      {
        power = m_dynamicPower / m_powerCounter;
      }

    return power;
  }

  double
  NocRouter::GetLeakagePower ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    MeasurePowerAndEnergyForLastClock ();
    double power = 0;
    NS_LOG_DEBUG ("Power was measured for " << m_powerCounter << " flits");
    if (m_powerCounter > 0)
      {
        power = m_leakagePower / m_powerCounter;
      }

    return power;
  }

  double
  NocRouter::GetTotalPower ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    MeasurePowerAndEnergyForLastClock ();
    double power = 0;
    NS_LOG_DEBUG ("Power was measured for " << m_powerCounter << " flits");
    if (m_powerCounter > 0)
      {
        power = (m_dynamicPower + m_leakagePower) / m_powerCounter;
      }

    return power;
  }

  double
  NocRouter::GetArea ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    double area = 0;

    // SIM_router_init(&GLOB(router_info), NULL, &GLOB(router_area));
    RouterInitForOrion(&GLOB(router_info), NULL, &GLOB(router_area));

    // area = SIM_router_area(&GLOB(router_area));
    area = GLOB(router_area).buffer + GLOB(router_area).crossbar + GLOB(router_area).vc_allocator
        + GLOB(router_area).sw_allocator;

#if( PARM(TECH_POINT) <= 90 )
    NS_LOG_LOGIC ("Buffer area: " << GLOB(router_area).buffer);
    NS_LOG_LOGIC ("Crossbar area: " << GLOB(router_area).crossbar);
    NS_LOG_LOGIC ("Virtual Channel allocator area: " << GLOB(router_area).vc_allocator);
    NS_LOG_LOGIC ("Switch allocator area: " << GLOB(router_area).sw_allocator);
#else
    NS_LOG_ERROR ("Router area is only supported for 90nm, 65nm, 45nm and 32nm");
#endif
    NS_LOG_LOGIC ("Router area is " << area << " um^2");
    return area;
  }

} // namespace ns3
