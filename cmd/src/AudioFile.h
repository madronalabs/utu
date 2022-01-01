//
// Copyright (c) 2021 Greg Wuller.
//
// SPDX-License-Identifier: GPL-2.0-or-later
//

#pragma once

#include <sndfile.h>

#include <filesystem>
#include <vector>

class AudioFile final
{
public:
    using Samples = std::vector<double>;

    enum Mode {
        READ,
        WRITE,
    };
    static AudioFile open(const std::filesystem::path& p, Mode m = Mode::READ);

    ~AudioFile();

    AudioFile(const AudioFile&) = delete;
    AudioFile& operator=(const AudioFile&) = delete;

    AudioFile(AudioFile&& other);
    AudioFile& operator=(AudioFile&& other);

    const std::filesystem::path& path() const { return _path; };
    Mode mode() const { return _mode; };

    const Samples& samples();

    int sampleRate() const;
    int channels() const;
    int64_t frames() const;

    void close();

private:
    AudioFile(const std::filesystem::path& p, Mode m) : _path(p), _mode(m), _file(nullptr) {};

    void _loadSamples();

    std::filesystem::path _path;
    Mode _mode;
    Samples _samples;

    SNDFILE *_file;
    SF_INFO _info;
};
