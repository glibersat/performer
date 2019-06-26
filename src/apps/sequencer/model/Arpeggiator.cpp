#include "Arpeggiator.h"
#include "ProjectVersion.h"

void Arpeggiator::clear() {
    setEnabled(false);
    setHold(false);
    setMode(Mode::PlayOrder);
    setDivisor(12);
    setGateLength(50);
    setOctaves(1);
}

void Arpeggiator::write(WriteContext &context) const {
    auto &writer = context.writer;
    writer.write(_enabled);
    writer.write(_hold);
    writer.write(_mode);
    writer.write(_divisor);
    writer.write(_gateLength);
    writer.write(_octaves);
}

void Arpeggiator::read(ReadContext &context) {
    auto &reader = context.reader;
    reader.read(_enabled, ProjectVersion::Version9);
    reader.read(_hold, ProjectVersion::Version9);
    reader.read(_mode, ProjectVersion::Version9);
    if (reader.dataVersion() < ProjectVersion::Version10) {
        reader.readAs<uint8_t>(_divisor, ProjectVersion::Version9);
    } else {
        reader.read(_divisor);
    }
    reader.read(_gateLength, ProjectVersion::Version9);
    reader.read(_octaves, ProjectVersion::Version9);
}