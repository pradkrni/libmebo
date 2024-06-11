#pragma once
#include <memory>
#include "../src/Handlers/AV1RateControlHandler.hpp"
#include "../src/Handlers/LibMeboControlHandler.hpp"
#include "../src/Handlers/VP8RateControlHandler.hpp"
#include "../src/Handlers/VP9RateControlHandler.hpp"

class Libmebo_brc_factory
{
  public: 
       static std::unique_ptr <Libmebo_brc> create(unsigned int id )
       {
         LibMeboCodecType codecType;
         switch(static_cast<LibMeboCodecType>(id))
         {
           case LIBMEBO_CODEC_VP8:
               codecType = LIBMEBO_CODEC_VP8;   
               break;
           case LIBMEBO_CODEC_VP9:
               codecType = LIBMEBO_CODEC_VP9;
               break;
           case LIBMEBO_CODEC_AV1:
               codecType = LIBMEBO_CODEC_AV1;
               break;
          }

         switch(codecType)
         {
             case LIBMEBO_CODEC_VP8:
                  return std::make_unique<Libmebo_brc_VP8>(static_cast<LibMeboBrcAlgorithmID>(id));//this is calling construcotr.
             case LIBMEBO_CODEC_VP9:
                  return std::make_unique<Libmebo_brc_VP9>(static_cast<LibMeboBrcAlgorithmID>(id));
              case LIBMEBO_CODEC_AV1:
                  return std::make_unique<Libmebo_brc_AV1>(static_cast<LibMeboBrcAlgorithmID>(id));
              case LIBMEBO_CODEC_UNKNOWN:
                  break;
         }

       }
};