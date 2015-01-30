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

		EntityProperty greater(EntityProperty greater);

		ValueType value() const { return mValue; }

		static EntityProperty max();
		static EntityProperty min();
	private:
		ValueType mValue;
};

#endif // ENTITYPROPERTY_H
