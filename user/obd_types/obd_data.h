#ifndef _OBD_DATA_H
#define _OBD_DATA_H

/* System header */
#include <string>
//#include <mutex>
//#include <atomic>
#include <vector>

enum OBDValueType
{
    OBD_VALUE_NUMERIC,
    OBD_VALUE_BOOLEAN
};

#define OBD_VALUE_DESCRIPTION_MAX_LENGTH 40
#define OBD_VALUE_SHORT_DESCRIPTION_LENGTH 4
#define OBD_VALUE_VALUE_UNIT_LENGTH 10


struct OBDValue
{
    unsigned char identifier;
    char description[OBD_VALUE_DESCRIPTION_MAX_LENGTH];
    char short_description[OBD_VALUE_SHORT_DESCRIPTION_LENGTH];
    OBDValueType type;
    float min;
    float max;
    float zero;
    char value_unit[OBD_VALUE_VALUE_UNIT_LENGTH];
    float value_f;
    bool value_b;
};

#if 0

class OBDData
{
public:
    OBDData(char identifier, const char description_short[4]);
    virtual ~OBDData();

	virtual OBDData* clone() const = 0;

    virtual const std::string& get_description() const;
    virtual const std::string& get_unit() const;

    virtual OBDValue get_value() const = 0;
    virtual int set_value(const OBDValue &value) = 0;

    int get_data_identifier() const;

protected:
	/* Copy constructor inaccessible to prevent slicing */
	OBDData(const OBDData& other);

    //std::string description;
	const char description_short[4];

    //std::string unit;

    /* Unique index for an OBD data. */
    char identifier;

};

class NumericOBDData: public OBDData
{
public:
	NumericOBDData(char identifier, const std::string &description, const char description_short[4], float min = 0.0f, float max = 0.0f, float zero = 0.0);

	/* Copy constructor */
	NumericOBDData(const NumericOBDData& other);

	virtual NumericOBDData* clone() const;

    virtual OBDValue get_value() const;
    virtual int set_value(const OBDValue &value);

    void set_value_f(float value);

    float get_min() const;
    float get_max() const;
    float get_zero() const;

private:
    float value; /* Should be thread-safe */
    float min;
    float max;
    float zero;

    mutable std::mutex value_mutex;
};

class BooleanOBDData: public OBDData
{
public:
	BooleanOBDData(char identifier, const std::string &description, const char description_short[4], bool value = false);

	/* Copy constructor */
	BooleanOBDData(const BooleanOBDData& other);

	virtual BooleanOBDData* clone() const;

    virtual OBDValue get_value() const;
    virtual int set_value(const OBDValue &value);
private:
    std::atomic<bool> value;
};
#endif

//#include <memory>

typedef std::vector<OBDValue> OBDDataList;



namespace OBDataListHelper
{
	bool is_obd_data_list_valid(const OBDDataList &obd_data);
	void clone(const OBDDataList &src, OBDDataList &dst);
	OBDDataList::iterator get_element_by_identifier(OBDDataList &src, uint8_t identifier);

}

bool operator==(const OBDValue &v1, const OBDValue &v2);
bool operator!=(const OBDValue &v1, const OBDValue &v2);

#endif  /* _OBD_DATA_H */

