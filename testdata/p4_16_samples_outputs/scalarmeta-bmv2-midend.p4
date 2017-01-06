#include <core.p4>
#include <v1model.p4>

header d {
    bit<8> f;
}

struct h {
}

struct m {
    bit<3> counter;
}

parser MyParser(packet_in b, out h hdrs, inout m meta, inout standard_metadata_t std) {
    bit<3> tmp_0;
    state start {
        meta.counter = 3w4;
        tmp_0 = meta.counter + 3w7;
        meta.counter = tmp_0;
        transition accept;
    }
}

control MyVerifyChecksum(in h hdr, inout m meta) {
    apply {
    }
}

control MyIngress(inout h hdr, inout m meta, inout standard_metadata_t std) {
    apply {
    }
}

control MyEgress(inout h hdr, inout m meta, inout standard_metadata_t std) {
    apply {
    }
}

control MyComputeChecksum(inout h hdr, inout m meta) {
    apply {
    }
}

control MyDeparser(packet_out b, in h hdr) {
    apply {
    }
}

V1Switch<h, m>(MyParser(), MyVerifyChecksum(), MyIngress(), MyEgress(), MyComputeChecksum(), MyDeparser()) main;
