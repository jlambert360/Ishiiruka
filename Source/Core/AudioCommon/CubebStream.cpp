// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <cubeb/cubeb.h>

#include "AudioCommon/CubebStream.h"
#include "AudioCommon/CubebUtils.h"
#include "AudioCommon/DPL2Decoder.h"
#include "Common/CommonTypes.h"
#include "Common/Logging/Log.h"
#include "Common/Thread.h"
#include "Core/ConfigManager.h"

// ~10 ms - needs to be at least 240 for surround
constexpr u32 BUFFER_SAMPLES = 512;

long CubebStream::DataCallback(cubeb_stream* stream, void* user_data, const void* /*input_buffer*/,
                               void* output_buffer, long num_frames)
{
  auto* self = static_cast<CubebStream*>(user_data);

  self->m_mixer->Mix(static_cast<short*>(output_buffer), num_frames);

  return num_frames;
}

void CubebStream::StateCallback(cubeb_stream* stream, void* user_data, cubeb_state state)
{
}

bool CubebStream::Init()
{
  m_ctx = CubebUtils::GetContext();
  if (!m_ctx)
    return false;

  cubeb_stream_params params;
  params.rate = m_mixer->GetSampleRate();
  params.channels = 2;
  params.format = CUBEB_SAMPLE_S16NE;
  params.layout = CUBEB_LAYOUT_STEREO;

  u32 minimum_latency = 0;
  if (cubeb_get_min_latency(m_ctx.get(), &params, &minimum_latency) != CUBEB_OK)
    ERROR_LOG(AUDIO, "Error getting minimum latency");
  INFO_LOG(AUDIO, "Minimum latency: %i frames", minimum_latency);

  return cubeb_stream_init(m_ctx.get(), &m_stream, "Dolphin Audio Output", nullptr, nullptr,
                           nullptr, &params, std::max(BUFFER_SAMPLES, minimum_latency),
                           DataCallback, StateCallback, this) == CUBEB_OK;
}

bool CubebStream::SetRunning(bool running)
{
  if (running)
    return cubeb_stream_start(m_stream) == CUBEB_OK;
  else
    return cubeb_stream_stop(m_stream) == CUBEB_OK;
}

CubebStream::~CubebStream()
{
  SetRunning(false);
  cubeb_stream_destroy(m_stream);
  m_ctx.reset();
}

void CubebStream::SetVolume(int volume)
{
  cubeb_stream_set_volume(m_stream, volume / 200.0f);
}
