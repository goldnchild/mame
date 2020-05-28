// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    PK-8020

    Disk image format

***************************************************************************/
#ifndef MAME_FORMATS_PK8020_DSK_H
#define MAME_FORMATS_PK8020_DSK_H

#pragma once

#include "wd177x_dsk.h"

class pk8020_format : public wd177x_format
{
public:
	pk8020_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_PK8020_FORMAT;

#endif // MAME_FORMATS_PK8020_DSK_H
