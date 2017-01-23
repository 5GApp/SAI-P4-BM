#include <iostream>

//SAI
#ifdef __cplusplus
extern "C" {
#endif
#include <sai.h>
#ifdef __cplusplus
}
#endif

#include <saifdb.h>
#include <saivlan.h>
#include <sairouter.h>
#include <sairouterintf.h>
#include <sairoute.h>
#include <saiswitch.h>
#include <saimirror.h>
#include <saistatus.h>

// INTERNAL
#include "switch_meta_data.h"

// thrift bm clinet
#include "../../../../thrift_src/gen-cpp/bm/Standard.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TMultiplexedProtocol.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TSocket.h>

using namespace std;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;

using namespace bm_runtime::standard;

const int bm_port = 9090;
const int32_t cxt_id =0;

static StandardClient* bm_client_ptr;
static sai_id_map_t* sai_id_map_ptr;
class sai_object {
public:
	boost::shared_ptr<TTransport> socket;
  	boost::shared_ptr<TTransport> transport;
  	boost::shared_ptr<TProtocol>  bprotocol;
  	boost::shared_ptr<TProtocol>  protocol;
  	StandardClient bm_client;

	static sai_status_t create_port (sai_object_id_t *port_id, sai_object_id_t switch_id,uint32_t attr_count,const sai_attribute_t *attr_list);
	sai_port_api_t port_api;
	sai_id_map_t sai_id_map;
    switch_metatdata_t switch_metatdata;  // TODO expand to array for multiple switch support
	

	sai_object():
	//  constructor pre initializations
	  socket(new TSocket("localhost", bm_port)),
	  transport(new TBufferedTransport(socket)),
	  bprotocol(new TBinaryProtocol(transport)),
	  protocol (new TMultiplexedProtocol(bprotocol, "standard")),
	  bm_client(protocol)
	  {
	  	uint32_t list[]={0,1,2,3,4,5,6,7};
  		switch_metatdata.hw_port_list.list=list;
  		switch_metatdata.hw_port_list.count=8;
  		extern StandardClient* bm_client_ptr;
  		bm_client_ptr = &bm_client;
  		extern sai_id_map_t* sai_id_map_ptr;
  		sai_id_map_ptr = &sai_id_map;
	  	transport->open();
  

  		port_api.create_port  = &sai_object::create_port;
    	
    	printf("BM connection started on port %d\n",bm_port); 
	  }
	~sai_object(){
	 	  //deconstructor
  		transport->close();
    	printf("BM clients closed\n");
	 }

	sai_status_t sai_api_query(sai_api_t sai_api_id,void** api_method_table){
		switch (sai_api_id) {
            case SAI_API_PORT:
              *api_method_table=&port_api;
              break;
         	default:
         		printf("api requested was %d, while sai_api_port is %d\n",sai_api_id,SAI_API_PORT);
         		//*api_method_table=&port_api; ///TODOOOOO remove!!
         		return SAI_STATUS_FAILURE; ///TODOOOOO enable!!
         	}
		return SAI_STATUS_SUCCESS;
	}
};

std::string parse_param(uint64_t param, uint32_t num_of_bytes) {
		return std::string(static_cast<char*>(static_cast<void*>(&param)),num_of_bytes);
	}

	BmMatchParam parse_exact_match_param(uint64_t param, uint32_t num_of_bytes) {
		BmMatchParam match_param;
		match_param.type = BmMatchParamType::type::EXACT; 
		BmMatchParamExact match_param_exact;
	    match_param_exact.key = parse_param(param, num_of_bytes);
	    match_param.__set_exact(match_param_exact);
	    return match_param;
	}

	sai_status_t sai_object::create_port (sai_object_id_t *port_id, sai_object_id_t switch_id,uint32_t attr_count,const sai_attribute_t *attr_list){
		printf("create port");
		Port_obj port(*sai_id_map_ptr);
		printf("port sai_id = %d\n",port.sai_object_id);
		BmEntryHandle handle = 0;
		//parsing attributes
		sai_attribute_t attribute;
		for(uint32_t i = 0; i < attr_count; i++) {
          attribute =attr_list[i];
          switch (attribute.id) {
          	case SAI_PORT_ATTR_PORT_VLAN_ID:
          		port.pvid = attribute.value.u16;
          	break;
          	case SAI_PORT_ATTR_BIND_MODE:
          		port.bind_mode = attribute.value.s32;
          	break;
          	case SAI_PORT_ATTR_HW_LANE_LIST:
          		port.hw_port = attribute.value.u32list.list[0];
          	break;
          }
      }
      	printf("pvid %d, bind_mode %d, hw_port %d \n", port.pvid, port.bind_mode, port.hw_port);
	    BmAddEntryOptions options;
		BmMatchParams match_params;
        BmActionData action_data;

        match_params.push_back(parse_exact_match_param(port.sai_object_id,1));
        action_data.push_back(parse_param(port.pvid,1));
        handle=bm_client_ptr->bm_mt_add_entry(cxt_id,"table_port_PVID",match_params, "action_set_pvid" ,action_data, options);
		printf("pvid handle= %d\n",handle);


		action_data.clear();
		action_data.push_back(parse_param(port.bind_mode,1));
        handle=bm_client_ptr->bm_mt_add_entry(cxt_id,"table_port_mode",match_params,  "action_set_port_mode", action_data, options);
		printf("port mode handle= %d\n",handle);


        action_data.clear();
        match_params.clear();
        match_params.push_back(parse_exact_match_param(port.hw_port,2));
        action_data.push_back(parse_param(0,1));
        action_data.push_back(parse_param(port.sai_object_id,1));
        handle=bm_client_ptr->bm_mt_add_entry(cxt_id,"table_ingress_lag",match_params, "action_set_lag_l2if",  action_data, options);
        printf("lag handle= %d",handle);
		*port_id = port.sai_object_id;

		return SAI_STATUS_SUCCESS;
	}