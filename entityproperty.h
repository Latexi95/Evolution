#ifndef ENTITYPROPERTY_H
#define ENTITYPROPERTY_H
#include <QtGlobal>

class EntityProperty {
	public:
		typedef quint32 CalcValueType;
		typedef quint16 ValueType;
		EntityProperty();
		EntityProperty(ValueType val);
		~EntityProperty();
		EntityProperty operator +(EntityProperty o) const;
		EntityProperty operator -(EntityProperty o) const;
		EntityProperty operator *(EntityProperty o) const;
		EntityProperty operator /(EntityProperty o) const;

		EntityProperty &operator += (EntityProperty o);
		EntityProperty &operator -= (EntityProperty o);

		EntityProperty logicalAnd(EntityProperty o) const;
		EntityProperty logicalOr(EntityProperty o) const;
		EntityProperty logicalNot() const;
		EntityProperty binarized() const;
		EntityProperty equal(EntityProperty o) const;
		bool operator > (EntityProperty o) const;
		bool operator < (EntityProperty o) const;
		bool operator == (EntityProperty o) const;
		bool operator != (EntityProperty o) const;
		bool isMin() const;
		bool isMax() const;
		EntityProperty take(EntityProperty target);
		EntityProperty sqrt() const;
		EntityProperty square() const;

		EntityProperty greater(EntityProperty greater);

		ValueType value() const { return mValue; }
		void setValue(ValueType val) { mValue = val; }

		static EntityProperty max();
		static EntityProperty min();
	private:
		ValueType mValue;
};

QDataStream &operator <<(QDataStream &out, const EntityProperty &p);
QDataStream &operator >>(QDataStream &in, EntityProperty &p);


inline EntityProperty EntityProperty::operator +(EntityProperty o) const {
	if (std::numeric_limits<ValueType>::max() - this->mValue < o.mValue) {
		return EntityProperty(std::numeric_limits<ValueType>::max());
	}
	return EntityProperty(this->mValue + o.mValue);
}

inline EntityProperty EntityProperty::operator -(EntityProperty o) const {
	if (this->mValue < o.mValue) {
		return EntityProperty(std::numeric_limits<ValueType>::min());
	}
	return EntityProperty(this->mValue - o.mValue);
}

inline EntityProperty EntityProperty::operator *(EntityProperty o) const {
	CalcValueType result = (CalcValueType)this->mValue * (CalcValueType)o.mValue;
	if (result > std::numeric_limits<ValueType>::max()) {
		return max();
	}
	else {
		return EntityProperty(result);
	}
}

inline EntityProperty EntityProperty::operator /(EntityProperty o) const {
	return this->mValue / o.mValue;
}

inline EntityProperty &EntityProperty::operator +=(EntityProperty o) {
	if (std::numeric_limits<ValueType>::max() - this->mValue < o.mValue) {
		this->mValue = std::numeric_limits<ValueType>::max();
	}
	else {
		this->mValue += o.mValue;
	}
	return *this;
}

inline EntityProperty &EntityProperty::operator -=(EntityProperty o) {
	if (this->mValue < o.mValue) {
		this->mValue = std::numeric_limits<ValueType>::min();
	}
	else {
		this->mValue -= o.mValue;
	}
	return *this;
}

inline EntityProperty EntityProperty::logicalAnd(EntityProperty o) const {
	return (*this != min() && o != min()) ? max() : min();
}

inline EntityProperty EntityProperty::logicalOr(EntityProperty o) const {
	return (*this != min() || o != min()) ? max() : min();
}

inline EntityProperty EntityProperty::logicalNot() const {
	return (*this == min()) ? max() : min();
}

inline EntityProperty EntityProperty::binarized() const {
	return (*this != min()) ? max() : min();
}
inline EntityProperty EntityProperty::equal(EntityProperty o) const {
	return (this->mValue == o.mValue) ? max() : min();
}



inline bool EntityProperty::operator >(EntityProperty o) const {
	return this->mValue > o.mValue;
}

inline bool EntityProperty::operator <(EntityProperty o) const {
	return this->mValue < o.mValue;
}

inline bool EntityProperty::operator ==(EntityProperty o) const {
	return this->mValue == o.mValue;
}

inline bool EntityProperty::operator !=(EntityProperty o) const {
	return this->mValue != o.mValue;
}

inline bool EntityProperty::isMin() const {
	return this->mValue == std::numeric_limits<ValueType>::min();
}

inline bool EntityProperty::isMax() const {
	return this->mValue == std::numeric_limits<ValueType>::max();
}

inline EntityProperty EntityProperty::take(EntityProperty target) {
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

inline EntityProperty EntityProperty::sqrt() const {
	return (ValueType)std::sqrt((float)this->mValue);
}

inline EntityProperty EntityProperty::greater(EntityProperty greater) {
	return this->mValue > greater.mValue ? this->mValue : greater.mValue;
}

inline EntityProperty EntityProperty::max() {
	return EntityProperty(std::numeric_limits<ValueType>::max());
}

inline EntityProperty EntityProperty::min() {
	return EntityProperty(std::numeric_limits<ValueType>::min());
}

inline EntityProperty EntityProperty::square() const {
	if (this->mValue < std::numeric_limits<ValueType>::max() / 2) {
		return this->mValue * this->mValue;
	}
	return max();
}

#endif // ENTITYPROPERTY_H
