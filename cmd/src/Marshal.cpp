//
// Copyright (c) 2021 Greg Wuller.
//
// SPDX-License-Identifier: GPL-2.0-or-later
//

#include "Marshal.h"

// TODO: move to Partial.h
constexpr char kTimeName[] = "time";
constexpr char kFrequencyName[] = "frequency";
constexpr char kAmplitudeName[] = "amplitude";
constexpr char kBandwidthName[] = "bandwidth";
constexpr char kPhaseName[] = "phase";

utu::PartialData Marshal::from(const Loris::PartialList& partials)
{
  utu::PartialData result;

  result.parameters = {
      kTimeName, kFrequencyName, kAmplitudeName, kBandwidthName, kPhaseName,
  };

  for (const auto& ip : partials) {
    utu::Partial::Envelope time;
    utu::Partial::Envelope frequency;
    utu::Partial::Envelope amplitude;
    utu::Partial::Envelope bandwidth;
    utu::Partial::Envelope phase;

    for (auto it = ip.begin(); it != ip.end(); it++) {
      time.push_back(it.time());
      frequency.push_back(it->frequency());
      amplitude.push_back(it->amplitude());
      bandwidth.push_back(it->bandwidth());
      phase.push_back(it->phase());
    }

    utu::Partial op;
    op.envelopes[kTimeName] = time;
    op.envelopes[kFrequencyName] = frequency;
    op.envelopes[kAmplitudeName] = amplitude;
    op.envelopes[kBandwidthName] = bandwidth;
    op.envelopes[kPhaseName] = phase;

    result.push_back(op);  // FIXME: check for errors
  }

  return result;
}

Loris::PartialList Marshal::from(const utu::PartialData& data)
{
  // TODO: validate the required parameters are present

  Loris::PartialList result;

  for (auto partial : data.partials) {
    auto times = partial.envelopes[kTimeName];
    auto frequencies = partial.envelopes[kFrequencyName];
    auto amplitudes = partial.envelopes[kAmplitudeName];
    auto bandwidths = partial.envelopes[kBandwidthName];
    auto phases = partial.envelopes[kPhaseName];

    auto t = times.cbegin();
    auto f = frequencies.cbegin();
    auto a = amplitudes.cbegin();
    auto b = bandwidths.cbegin();
    auto p = phases.cbegin();

    Loris::Partial out;

    for (; t != times.end() and f != frequencies.end() and a != amplitudes.end() and
           b != bandwidths.end() and p != phases.end();
         ++t, ++f, ++a, ++b, ++p) {
      out.insert(*t, Loris::Breakpoint(*f, *a, *b, *p));
    }

    result.push_back(out);
  }

  return result;
}
