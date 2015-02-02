#include "entityproperty.h"
#include <limits>
#include <cmath>
#include <QDataStream>

EntityProperty::EntityProperty() :
	mValue(0) {

}

EntityProperty::EntityProperty(EntityProperty::ValueType val) :
	mValue(val) {

}

EntityProperty::~EntityProperty() {

}




QDataStream &operator << (QDataStream &out, const EntityProperty &p) {
	out << p.value();
	return out;
}


QDataStream &operator >>(QDataStream &in, EntityProperty &p) {
	EntityProperty::ValueType val;
	in >> val;
	p.setValue(val);
	return in;
}
