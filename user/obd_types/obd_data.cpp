
/* System header */
#include <string>




/* Own header */
#include "obd_types/obd_data.h"
#if 0
OBDData::OBDData(char identifier, const char description_short[4])
        : identifier(identifier), description_short("HAL")
{
    //debug_log("Data" + this->get_description() + " is being destroyed!");
}

OBDData::~OBDData()
{
    //debug_log("Data" + this->get_description() + " is being destroyed!");
}

int OBDData::get_data_identifier() const
{
    return this->identifier;
}

const std::string& OBDData::get_description() const
{
    return""; // this->description;
}

const std::string& OBDData::get_unit() const
{
    return ""; //this->unit;
}

NumericOBDData::NumericOBDData(char identifier, const std::string &description, const char description_short[4], float min, float max, float zero)
: OBDData(identifier, description_short), min(min), max(max), zero(zero), value(0.0f)
{}

NumericOBDData::NumericOBDData(const NumericOBDData& other)
: OBDData(other.identifier, other.description_short), min(other.min), max(other.max), zero(other.zero), value(other.value)
{}

NumericOBDData* NumericOBDData::clone() const
{
	return new NumericOBDData(*this);
}


OBDValue NumericOBDData::get_value() const
{
    OBDValue ret_val;
    ret_val.description = ""; //this->description;
    ret_val.max = this->max;
    ret_val.min = this->min;
    ret_val.zero = this->zero;
    this->value_mutex.lock();
    ret_val.value_f = this->value;
    this->value_mutex.unlock();
    ret_val.type = OBD_VALUE_NUMERIC;
    return ret_val;
}

int NumericOBDData::set_value(const OBDValue &value)
{
	this->set_value_f(value.value_f);
}

void NumericOBDData::set_value_f(float value)
{
    this->value_mutex.lock();
    this->value = value;
    this->value_mutex.unlock();
}

float NumericOBDData::get_min() const { return this->min; }
float NumericOBDData::get_max() const { return this->max; }
float NumericOBDData::get_zero() const { return this->zero; }


/* Copy constructor */
BooleanOBDData::BooleanOBDData(const BooleanOBDData& other)
: OBDData(other.identifier, other.description_short)
{}

BooleanOBDData* BooleanOBDData::clone() const
{
	return new BooleanOBDData(*this);
}

OBDValue BooleanOBDData::get_value() const
{
    OBDValue ret_val;
    ret_val.description ="";
    ret_val.value_b = this->value;
    ret_val.type = OBD_VALUE_BOOLEAN;
    return ret_val;
}

int BooleanOBDData::set_value(const OBDValue &value)
{
	this->value = value.value_b;
	return 0;
}


bool OBDataListHelper::is_obd_data_list_valid(const OBDDataList &obd_data)
{
	return true;
}

void OBDataListHelper::clone(const OBDDataList &src, OBDDataList &dst)
{
	dst.clear();
	dst.reserve(src.size());
	for (const std::shared_ptr<OBDData> &obd_data : src)
	{
		dst.push_back(std::shared_ptr<OBDData>(obd_data->clone()));
	}
}
#endif

bool OBDataListHelper::is_obd_data_list_valid(const OBDDataList &obd_data)
{
	return true;
}

void OBDataListHelper::clone(const OBDDataList &src, OBDDataList &dst)
{
	dst = src;
}

bool operator==(const OBDValue &v1, const OBDValue &v2)
{
	if (v1.type != v2.type)
	{
		return false;
	}

	switch (v1.type)
	{
	case OBD_VALUE_NUMERIC:
		return v1.value_f == v2.value_f;
	case OBD_VALUE_BOOLEAN:
		return v1.value_b == v2.value_b;
	}
	return false;
}

bool operator!=(const OBDValue &v1, const OBDValue &v2)
{
	return !(v1 == v2);
}
