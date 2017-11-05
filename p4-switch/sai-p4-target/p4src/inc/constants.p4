const bit<16> MAC_LEARN_TRAP_ID = 512;
const bit<9> COPY_TO_CPU_MIRROR_ID = 250;
const bit<2>  NETDEV_TYPE_PORT = 0;
const bit<2>  NETDEV_TYPE_LAG  = 1;
const bit<2>  NETDEV_TYPE_VLAN = 2;
const bit PORT_MODE_SUBPORT = 0x1;
const bit PORT_MODE_PORT    = 0x0;
const bit<2> OUT_IF_PORT = 0;
const bit<2> OUT_IF_LAG = 1;
const bit<2> OUT_IF_ROUTER = 2;
const bit<2> L2_IF_ROUTER_TYPE = 1;
const bit<2> L2_IF_1D_BRIDGE = 2;
const bit<2> L2_IF_1Q_BRIDGE = 3;