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

NS_LOG_COMPONENT_DEFINE ("NocRouter");

namespace ns3
{

  NS_OBJECT_ENSURE_REGISTERED (NocRouter);

  NocRouter::NocRouter (std::string name)
  {
    m_name = name;
    m_loadComponent = 0;
    NS_LOG_DEBUG ("No load router component is used");
  }

  NocRouter::NocRouter (std::string name, Ptr<LoadRouterComponent> loadComponent)
  {
    NS_ASSERT_MSG (loadComponent != 0, "The load router component must be specified!"
        " If you do not want to use a load router component, use another constructor.");
    m_name = name;
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
  NocRouter::ManagePacket (const Ptr<NocNetDevice> source, const Ptr<NocNode> destination, Ptr<Packet> packet)
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
        route = GetRoutingProtocol()->RequestRoute (source, destination, packet);
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
  NocRouter::routerInitForOrion (SIM_router_info_t *info, SIM_router_power_t *router_power, SIM_router_area_t *router_area)
  {
    u_int line_width;
    int share_buf, outdrv;

    /* PHASE 1: set parameters */
    /* general parameters */
    info->n_in = PARM(in_port);
    info->n_cache_in = PARM(cache_in_port);
    info->n_mc_in = PARM(mc_in_port);
    info->n_io_in = PARM(io_in_port);
    info->n_total_in = PARM(in_port) + PARM(cache_in_port) + PARM(mc_in_port) + PARM(io_in_port);
    info->n_out = PARM(out_port);
    info->n_cache_out = PARM(cache_out_port);
    info->n_mc_out = PARM(mc_out_port);
    info->n_io_out = PARM(io_out_port);
    info->n_total_out = PARM(out_port) + PARM(cache_out_port) + PARM(mc_out_port) + PARM(io_out_port);
    info->flit_width = PARM(flit_width);

    /* virtual channel parameters */
    info->n_v_channel = MAX(PARM(v_channel), 1);
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
    info->in_buf = PARM(in_buf);
    info->in_buffer_model = PARM(in_buffer_type);
    if(info->in_buf){
            outdrv = !info->in_share_buf && info->in_share_switch;
            SIM_array_init(&info->in_buf_info, 1, PARM(in_buf_rport), 1, PARM(in_buf_set), PARM(flit_width), outdrv, info->in_buffer_model);
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
            SIM_array_init(&info->cache_in_buf_info, 1, PARM(cache_in_buf_rport), 1, PARM(cache_in_buf_set), PARM(flit_width), outdrv, SRAM);
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
            SIM_array_init(&info->mc_in_buf_info, 1, PARM(mc_in_buf_rport), 1, PARM(mc_in_buf_set), PARM(flit_width), outdrv, SRAM);
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
            SIM_array_init(&info->io_in_buf_info, 1, PARM(io_in_buf_rport), 1, PARM(io_in_buf_set), PARM(flit_width), outdrv, SRAM);
            }
    }

    /* output buffer */
    info->out_buf = PARM(out_buf);
    info->out_buffer_model = PARM(out_buffer_type);
    if (info->out_buf){
            /* output buffer has no tri-state buffer anyway */
            SIM_array_init(&info->out_buf_info, 1, 1, PARM(out_buf_wport), PARM(out_buf_set), PARM(flit_width), 0, info->out_buffer_model);
    }

    /* central buffer */
    info->central_buf = PARM(central_buf);
    if (info->central_buf){
            info->pipe_depth = PARM(pipe_depth);
            /* central buffer is no FIFO */
            SIM_array_init(&info->central_buf_info, 0, PARM(cbuf_rport), PARM(cbuf_wport), PARM(cbuf_set), PARM(cbuf_width) * PARM(flit_width), 0, SRAM);
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
  NocRouter::GetArea ()
  {
    NS_LOG_FUNCTION_NOARGS ();
    double area = 0;

    routerInitForOrion(&GLOB(router_info), NULL, &GLOB(router_area));

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
