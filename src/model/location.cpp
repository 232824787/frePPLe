/***************************************************************************
  file : $URL$
  version : $LastChangedRevision$  $LastChangedBy$
  date : $LastChangedDate$
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 * Copyright (C) 2007-2010 by Johan De Taeye                                    *
 *                                                                         *
 * This library is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation; either version 2.1 of the License, or  *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser *
 * General Public License for more details.                                *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,*
 * USA                                                                     *
 *                                                                         *
 ***************************************************************************/

#define FREPPLE_CORE
#include "frepple/model.h"

namespace frepple
{

template<class Location> DECLARE_EXPORT Tree utils::HasName<Location>::st;
DECLARE_EXPORT const MetaCategory* Location::metadata;
DECLARE_EXPORT const MetaClass* LocationDefault::metadata;


int Location::initialize()
{
  // Initialize the metadata
  metadata = new MetaCategory("location", "locations", reader, writer);

  // Initialize the Python class
  return FreppleCategory<Location>::initialize();
}


int LocationDefault::initialize()
{
  // Initialize the metadata
  LocationDefault::metadata = new MetaClass("location", "location_default",
    Object::createString<LocationDefault>, true);

  // Initialize the Python class
  return FreppleClass<LocationDefault,Location>::initialize();
}


DECLARE_EXPORT void Location::writeElement(XMLOutput* o, const Keyword& tag, mode m) const
{
  // Writing a reference
  if (m == REFERENCE)
  {
    o->writeElement(tag, Tags::tag_name, getName());
    return;
  }

  // Write the complete object
  if (m != NOHEADER) o->BeginObject(tag, Tags::tag_name, getName());

  // Write the fields
  HasDescription::writeElement(o, tag);
  HasHierarchy<Location>::writeElement(o, tag);
  o->writeElement(Tags::tag_available, available);
  o->EndObject(tag);
}


DECLARE_EXPORT void Location::beginElement(XMLInput& pIn, const Attribute& pAttr)
{
  if (pAttr.isA(Tags::tag_available) || pAttr.isA(Tags::tag_maximum))
    pIn.readto( Calendar::reader(Calendar::metadata,pIn.getAttributes()) );
  else
    HasHierarchy<Location>::beginElement(pIn, pAttr);
}


DECLARE_EXPORT void Location::endElement(XMLInput& pIn, const Attribute& pAttr, const DataElement& pElement)
{
  if (pAttr.isA(Tags::tag_available))
  {
    CalendarBool *cal = dynamic_cast<CalendarBool*>(pIn.getPreviousObject());
    if (cal)
      setAvailable(cal);
    else
    {
      Calendar *c = dynamic_cast<Calendar*>(pIn.getPreviousObject());
      if (!c)
        throw LogicException("Incorrect object type during read operation");
      throw DataException("Calendar '" + c->getName() +
          "' has invalid type for use as location calendar");
    }
  }
  else
  {
    HasDescription::endElement(pIn, pAttr, pElement);
    HasHierarchy<Location>::endElement(pIn, pAttr, pElement);
  }
}


DECLARE_EXPORT Location::~Location()
{
  // Remove all references from buffers to this location
  for (Buffer::iterator buf = Buffer::begin();
      buf != Buffer::end(); ++buf)
    if (buf->getLocation() == this) buf->setLocation(NULL);

  // Remove all references from resources to this location
  for (Resource::iterator res = Resource::begin();
      res != Resource::end(); ++res)
    if (res->getLocation() == this) res->setLocation(NULL);

  // Remove all references from operations to this location
  for (Operation::iterator oper = Operation::begin();
      oper != Operation::end(); ++oper)
    if (oper->getLocation() == this) oper->setLocation(NULL);
}


DECLARE_EXPORT PyObject* Location::getattro(const Attribute& attr)
{
  if (attr.isA(Tags::tag_name))
    return PythonObject(getName());
  if (attr.isA(Tags::tag_description))
    return PythonObject(getDescription());
  if (attr.isA(Tags::tag_category))
    return PythonObject(getCategory());
  if (attr.isA(Tags::tag_subcategory))
    return PythonObject(getSubCategory());
  if (attr.isA(Tags::tag_owner))
    return PythonObject(getOwner());
  if (attr.isA(Tags::tag_available))
    return PythonObject(getAvailable());
  if (attr.isA(Tags::tag_hidden))
    return PythonObject(getHidden());
	return NULL;
}


DECLARE_EXPORT int Location::setattro(const Attribute& attr, const PythonObject& field)
{
  if (attr.isA(Tags::tag_name))
    setName(field.getString());
  else if (attr.isA(Tags::tag_description))
    setDescription(field.getString());
  else if (attr.isA(Tags::tag_category))
    setCategory(field.getString());
  else if (attr.isA(Tags::tag_subcategory))
    setSubCategory(field.getString());
  else if (attr.isA(Tags::tag_owner))
  {
    if (!field.check(Location::metadata))
    {
      PyErr_SetString(PythonDataException, "location owner must be of type location");
      return -1;
    }
    Location* y = static_cast<Location*>(static_cast<PyObject*>(field));
    setOwner(y);
  }
  else if (attr.isA(Tags::tag_available))
  {
    if (!field.check(CalendarBool::metadata))
    {
      PyErr_SetString(PythonDataException, "location calendar must be of type calendar_bool");
      return -1;
    }
    CalendarBool* y = static_cast<CalendarBool*>(static_cast<PyObject*>(field));
    setAvailable(y);
  }
  else if (attr.isA(Tags::tag_hidden))
    setHidden(field.getBool());
  else
    return -1;
  return 0;
}

} // end namespace