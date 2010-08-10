/*
 * noc-mapping-reader.cc
 *
 *  Created on: Aug 10, 2010
 *      Author: cipi
 */

#include <iostream>
#include "../../../CTG-XML/src/ro/ulbsibiu/acaps/ctg/xml/mapping/mapping.hxx"

using namespace std;
using namespace mapping;
using xml_schema::flags;

int
main (int argc, char* argv[])
{
  try
  {
    const string xmlFile("../Mapper/xml/e3s/auto-indust-mocsyn.tgff/ctg-0/mapping-0.xml");
    auto_ptr<mappingType> mt (mapping::mapping (xmlFile, flags::dont_validate));

    cerr << mt->id() << endl;

  }
  catch (const xml_schema::exception& e)
  {
    cerr << e << endl;
    return 1;
  }
}
