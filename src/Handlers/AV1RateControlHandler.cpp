#include <iostream>
#include <dlfcn.h>
#include "AV1RateControlHandler.hpp"

#include "../../aom/av1/ratectrl_rtc.h"



Libmebo_brc_AV1::Libmebo_brc_AV1(LibMeboBrcAlgorithmID algo_id) : Libmebo_brc( LIBMEBO_CODEC_AV1, algo_id), 
handle(nullptr), ptrCreateAV1Controller(nullptr)
{
  enc_params_libmebo.num_sl = 1;
	enc_params_libmebo.num_tl = 1;
	enc_params_libmebo.bitrate = 288; //in kbps.
	enc_params_libmebo.dynamic_rate_change =0;
	enc_params_libmebo.framecount = 100;
	enc_params_libmebo.framerate = 60;
	enc_params_libmebo.width = 320;
	enc_params_libmebo.height = 160;
    enc_params_libmebo.id = static_cast<unsigned int>( LIBMEBO_CODEC_AV1);
	enc_params_libmebo.preset =0;
	enc_params_libmebo.buf_optimal_sz = 600;
}

 Libmebo_brc_AV1::~Libmebo_brc_AV1()
 {
  if (handle)
  {
    dlclose(handle);
  }
 }

typedef   aom::AV1RateControlRtcConfig * (* create_av1_rate_control_config_t)();

aom::AV1RateControlRtcConfig *config ;
void *controller;
LibMeboRateController* Libmebo_brc_AV1::init(LibMeboRateController* libmebo_rc,
	LibMeboRateControllerConfig* libmebo_rc_config)
{
  LibMeboStatus status = LIBMEBO_STATUS_SUCCESS;
  

	libmebo_rc = Libmebo_brc::init (libmebo_rc, libmebo_rc_config);

    //  av1_rc_rtc_ = aom::AV1RateControlRTC::Create(rc_config_aom);
   // av1_rc_rtc_ = nullptr;
   // if(!av1_rc_rtc_)
   //   std::cout<<"FAiled to create AV1RateControlRTC \n";

    handle = dlopen("/home/pradeep4/Documents/pradeep/libmebo_git_submodule/libmebo/aom/build/libaom_av1_rc.so", RTLD_LAZY);
    if(!handle)
    {
      throw std::runtime_error("Cannot open the library ibaom_av1_rc.so");
    }

    ptrCreateAV1Controller = (createAV1Controller_t) dlsym(handle, "createAV1Controller");
    if(!ptrCreateAV1Controller)
    {
      throw std::runtime_error("Cannot load symbol  'createAV1Controller' from AOM LIB");
    }

    create_av1_rate_control_config_t create_av1_rate_control_config = 
     (create_av1_rate_control_config_t) dlsym(handle, "create_av1_rate_control_config");

    if(!create_av1_rate_control_config)
    {
      throw std::runtime_error("Cannot load symbol  'create_av1_rate_control_config' from AOM LIB");
    }
    std::cout<<"successfully loaded create, and config\n";
    config = create_av1_rate_control_config();

    constexpr int kMinQP = 10;
    constexpr int kMaxQP = 56;
    config->width = 640;
    config->height = 480;
    // third_party/webrtc/modules/video_coding/codecs/av1/libaom_av1_encoder.cc
    config->max_quantizer = kMaxQP;
    config->min_quantizer = kMinQP;

    config->buf_initial_sz = 600;
    config->buf_optimal_sz = 500;
    config->target_bandwidth =800000 /1000;
    config->buf_sz = 1000;
    config->undershoot_pct = 25;
    config->overshoot_pct = 50;
    config->max_intra_bitrate_pct = 300;
    config->max_inter_bitrate_pct = 50;
    config->framerate = 60;
    config->layer_target_bitrate[0] = 800000/1000;
    
    config->ts_rate_decimator[0] = 1;
    config->aq_mode = 0;
    config->ss_number_layers = 1;
    config->ts_number_layers = 1;
    config->max_quantizers[0] = kMaxQP;
    config->min_quantizers[0] = kMinQP;
    config->scaling_factor_num[0] = 1;
    config->scaling_factor_den[0] = 1;


    controller = ptrCreateAV1Controller(*config);
  std::cout<<"successfully loaded create, and config-1\n";

	return libmebo_rc;
} 
LibMeboStatus Libmebo_brc_AV1::update_config(LibMeboRateController* rc,
	LibMeboRateControllerConfig* rc_config)
{
  LibMeboStatus status = LIBMEBO_STATUS_SUCCESS;

  if (!rc || !rc_config)
    return status;
 // av1_rc_rtc_->UpdateRateControl(config);
	ptrUpdateRateControl_AV1 = (UpdateRateControl_AV1_t)dlsym(handle, "UpdateRateControl_AV1");
  if(!ptrUpdateRateControl_AV1)
  {
    throw std::runtime_error("Cannot load symbol  'UpdateRateControl_AV1' from AOM LIB");
  }

  bool success = ptrUpdateRateControl_AV1(controller, *config);

  
  return status;
}



LibMeboStatus Libmebo_brc_AV1::post_encode_update(LibMeboRateController* rc, uint64_t encoded_frame_size)
{
  LibMeboStatus status = LIBMEBO_STATUS_SUCCESS;

  if (!rc)
    return status;
  //av1_rc_rtc_->PostEncodeUpdate(encoded_frame_size);

  ptrPostEncodeUpdate_AV1 = (PostEncodeUpdate_AV1_t)dlsym(handle, "PostEncodeUpdate_AV1");
  if(!ptrPostEncodeUpdate_AV1)
  {
    throw std::runtime_error("Cannot load symbol  'PostEncodeUpdate_AV1' from AOM LIB");
  }

  ptrPostEncodeUpdate_AV1(controller, encoded_frame_size);

  return status;
}

LibMeboStatus Libmebo_brc_AV1::compute_qp(LibMeboRateController *rc, LibMeboRCFrameParams* rc_frame_params)
{
  LibMeboStatus status = LIBMEBO_STATUS_SUCCESS;
  if (!rc)
    return status;
  aom::AV1FrameParamsRTC frame_params_aom;
  
  if(rc_frame_params->frame_type == LIBMEBO_KEY_FRAME)
     frame_params_aom.frame_type = aom::kKeyFrame;
  else
    frame_params_aom.frame_type = aom::kInterFrame;
 //  av1_rc_rtc_->ComputeQP(frame_params_aom);

  ptrComputeQP_AV1 = (ComputeQP_AV1_t)dlsym(handle, "ComputeQP_AV1");
  if(!ptrComputeQP_AV1)
  {
    throw std::runtime_error("Cannot load symbol  'ComputeQP_AV1' from AOM LIB");
  }
  ptrComputeQP_AV1(controller, frame_params_aom);
  return status;
}

LibMeboStatus Libmebo_brc_AV1::get_qp(LibMeboRateController* rc, int* qp)
{
  LibMeboStatus status = LIBMEBO_STATUS_SUCCESS;
  
  if (!rc)
    return status;
 //  *qp =   (av1_rc_rtc_->GetQP());

  ptrGetQP_AV1 = (GetQP_AV1_t)dlsym(handle, "GetQP_AV1");
  if(!ptrGetQP_AV1)
  {
    throw std::runtime_error("Cannot load symbol  'GetQP_AV1' from AOM LIB");
  }

  *qp  = ptrGetQP_AV1(controller);
  return status;
}

LibMeboStatus Libmebo_brc_AV1::get_loop_filter_level(LibMeboRateController* rc, int* filter_level)
{
	LibMeboStatus status = LIBMEBO_STATUS_SUCCESS;
	if (!rc)
		return status;

    aom::AV1LoopfilterLevel loop_filter_level;
   // loop_filter_level = av1_rc_rtc_->GetLoopfilterLevel();

  ptrGetLoopfilterLevel_AV1 = (GetLoopfilterLevel_AV1_t)dlsym(handle, "GetLoopfilterLevel_AV1");
  if(!ptrGetLoopfilterLevel_AV1)
  {
    throw std::runtime_error("Cannot load symbol  'GetLoopfilterLevel_AV1' from AOM LIB");
  }

  loop_filter_level = ptrGetLoopfilterLevel_AV1(controller);


   filter_level[0] = loop_filter_level.filter_level[0];
   filter_level[1] = loop_filter_level.filter_level[1];
   filter_level[2] = loop_filter_level.filter_level_u;
   filter_level[3] = loop_filter_level.filter_level_v;

	return status;
}
