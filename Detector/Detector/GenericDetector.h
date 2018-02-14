#pragma once
#include "stdafx.h"

// Header class to link dll and executable files
class GenericPreparation {
public:
	virtual ~GenericPreparation() { ; }
	virtual void run() = 0;	
};
