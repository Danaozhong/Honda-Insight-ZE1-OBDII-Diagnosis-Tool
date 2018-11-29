//
// Created by Clemens on 12.12.2017.
//

#ifndef _OBD_ERROR_CODE_H
#define _OBD_ERROR_CODE_H

#include <string>
#include <vector>

#define OBD_ERROR_CODE_DESCRIPTION_MAX_LENGTH 100

struct OBDErrorCode
{
	unsigned int code_no;
	char description[OBD_ERROR_CODE_DESCRIPTION_MAX_LENGTH];
};

typedef std::vector<OBDErrorCode> OBDErrorCodeList;

#endif  /* _OBD_ERROR_CODE_H */
