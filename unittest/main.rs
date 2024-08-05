extern crate tinyproto;

use tinyproto::test;
use tinyproto::proto::hdlc::low_level::hdlc::sub;

fn main() {
    test();
    sub(3,1);
}
