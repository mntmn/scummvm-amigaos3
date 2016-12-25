#ifndef MIXER_AMIGAOS3_H
#define MIXER_AMIGAOS3_H

#include "audio/mixer_intern.h"

class AmigaOS3MixerManager {
public:
	AmigaOS3MixerManager();
	virtual ~AmigaOS3MixerManager();

	virtual void init(int priority);

	Audio::Mixer* getMixer();
};

#endif
