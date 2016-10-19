#ifndef COUPLINGDATAWRITER_H
#define COUPLINGDATAWRITER_H

#include "../CouplingDataUser.h"


namespace adapter
{
class CouplingDataWriter : public CouplingDataUser
{
protected:

	std::string _direction = "out";

public:

	CouplingDataWriter();
	virtual void write( double * dataBuffer ) = 0;
	virtual ~CouplingDataWriter()
	{
	}

};

}

#endif // COUPLINGDATAWRITER_H
