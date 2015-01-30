#include "entityproperty.h"
#include <limits>
#include <cmath>

EntityProperty::EntityProperty() :
	mValue(0) {

}

EntityProperty::EntityProperty(EntityProperty::ValueType val) :
	mValue(val) {

}

EntityProperty::~EntityProperty() {

}

EntityProperty EntityProperty::operator +(EntityProperty o) const {
	if (std::numeric_limits<ValueType>::max() - this->mValue < o.mValue) {
		return EntityProperty(std::numeric_limits<ValueType>::max());
	}
	return EntityProperty(this->mValue + o.mValue);
}

EntityProperty EntityProperty::operator -(EntityProperty o) const {
	if (this->mValue < o.mValue) {
		return EntityProperty(std::numeric_limits<ValueType>::min());
	}
	return EntityProperty(this->mValue - o.mValue);
}

EntityProperty EntityProperty::operator *(EntityProperty o) const {
	CalcValueType result = (CalcValueType)this->mValue * (CalcValueType)o.mValue;
	if (result > std::numeric_limits<ValueType>::max()) {
		return max();
	}
	else {
		return EntityProperty(result);
	}
}

EntityProperty EntityProperty::operator /(EntityProperty o) const {
	return this->mValue / o.mValue;
}

EntityProperty &EntityProperty::operator +=(EntityProperty o) {
	if (std::numeric_limits<ValueType>::max() - this->mValue < o.mValue) {
		this->mValue = std::numeric_limits<ValueType>::max();
	}
	else {
		this->mValue += o.mValue;
	}
	return *this;
}

EntityProperty &EntityProperty::operator -=(EntityProperty o) {
	if (this->mValue < o.mValue) {
		this->mValue = std::numeric_limits<ValueType>::min();
	}
	else {
		this->mValue -= o.mValue;
	}
	return *this;
}

EntityProperty EntityProperty::logicalAnd(EntityProperty o) const {
	return (*this != min() && o != min()) ? max() : min();
}

EntityProperty EntityProperty::logicalOr(EntityProperty o) const {
	return (*this != min() || o != min()) ? max() : min();
}

EntityProperty EntityProperty::logicalNot() const {
	return (*this == min()) ? max() : min();
}

EntityProperty EntityProperty::binarized() const {
	return (*this != min()) ? max() : min();
}
EntityProperty EntityProperty::equal(EntityProperty o) const {
	return (this->mValue == o.mValue) ? max() : min();
}



bool EntityProperty::operator >(EntityProperty o) const {
	return this->mValue > o.mValue;
}

bool EntityProperty::operator <(EntityProperty o) const {
	return this->mValue < o.mValue;
}

bool EntityProperty::operator ==(EntityProperty o) const {
	return this->mValue == o.mValue;
}
bool EntityProperty::operator !=(EntityProperty o) const {
	return this->mValue != o.mValue;
}

bool EntityProperty::isMin() const {
	return this->mValue == std::numeric_limits<ValueType>::min();
}

bool EntityProperty::isMax() const {
	return this->mValue == std::numeric_limits<ValueType>::max();
}

EntityProperty EntityProperty::take(EntityProperty target) {
	EntityProperty ret;
	if (this->mValue < target.mValue) {
		ret.mValue = this->mValue;
		this->mValue = std::numeric_limits<ValueType>::min();
	}
	else {
		ret.mValue = target.mValue;
		this->mValue -= target.mValue;
	}
	return ret;
}

EntityProperty EntityProperty::sqrt() const {
	return (ValueType)std::sqrt(this->mValue);
}

EntityProperty EntityProperty::greater(EntityProperty greater) {
	return this->mValue > greater.mValue ? this->mValue : greater.mValue;
}

EntityProperty EntityProperty::max() {
	return EntityProperty(std::numeric_limits<ValueType>::max());
}

EntityProperty EntityProperty::min() {
	return EntityProperty(std::numeric_limits<ValueType>::min());
}

