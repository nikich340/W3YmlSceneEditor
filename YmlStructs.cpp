#include "YmlStructs.h"

YAML::Emitter& YAML::operator <<(Emitter& out, const QString& s) {
    out << s.toStdString();
    return out;
}

YAML::Emitter& YAML::operator <<(Emitter& out, const ymlCond& cond) {
    out << Flow;
    out << BeginSeq << cond.condFact << cond.condOperand << cond.condValue << EndSeq;
    return out;
}
