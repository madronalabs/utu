
//
// Copyright (c) 2021 Greg Wuller.
//
// SPDX-License-Identifier: GPL-2.0-or-later
//

#include "AudioFile.h"

#include <cassert>
#include <limits>

#include <iostream>  // REMOVE

AudioFile AudioFile::open(const std::filesystem::path& p, Mode m)
{
    AudioFile file(p, m);

    int mode;
    switch (m) {
    case Mode::READ:
        mode = SFM_READ;
        break;
    case Mode::WRITE:
        mode = SFM_WRITE;
        break;
    default:
        mode = SFM_READ;
        break;
    }

    file._file = sf_open(p.c_str(), mode, &file._info);
    // FIXME: need proper error handling
    assert(file._file != nullptr);

    return file;
}

AudioFile::~AudioFile()
{
    close();
}

AudioFile::AudioFile(AudioFile&& other)
{
    _path = other._path;
    _mode = other._mode;
    _file = other._file;
    _info = other._info;

    other._file = nullptr;
    memset(&other._info, 0, sizeof(SF_INFO));
}

AudioFile& AudioFile::operator=(AudioFile&& other)
{
    if (this != &other) {
        // close any file handle which might be open in this instance
        close();

        _path = other._path;
        _mode = other._mode;
        _file = other._file;
        _info = other._info;

        other._file = nullptr;
        memset(&other._info, 0, sizeof(SF_INFO));
    }

    return *this;
}

const std::vector<double>& AudioFile::samples()
{
    if (_mode == Mode::READ && _samples.size() == 0) {
        _loadSamples();
    }
    return _samples;
}

int AudioFile::sampleRate() const
{
    return _file ? _info.samplerate : 0;
}

int AudioFile::channels() const
{
    return _file ? _info.channels : 0;
}

int64_t AudioFile::frames() const
{
    return _file ? _info.frames : 0;
}


void AudioFile::close()
{
    if (_file) {
        // FIXME: check for errors
        sf_close(_file);
    }
}

void AudioFile::_loadSamples()
{
    if (_file) {
        // TODO: generalize to handle files with multiple channels
        assert(_info.channels == 1);

        // ensure the down cast to reserve size will not overflow
        assert(_info.frames >= 0);
        assert(std::numeric_limits<sf_count_t>::max() <= std::numeric_limits<Samples::size_type>::max());

        // assign (as opposed to reserve) so that the vector size reflects the size of the data being written into the backing memory
        _samples.assign(static_cast<Samples::size_type>(_info.frames), 0.0);

        sf_count_t read = sf_read_double(_file, _samples.data(), static_cast<sf_count_t>(_samples.capacity()));
        assert(read == _info.frames);
    }
}