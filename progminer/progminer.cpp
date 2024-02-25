#include "ethash/ethash.hpp"
#include "libdevcore/FixedHash.h"
#include "libethcore/Miner.h"
#include "libethcore/Farm.h"
#include <algorithm>
#if ETH_ETHASHCL
#include <libethash-cl/CLMiner.h>
#endif
#if ETH_ETHASHCUDA
#include <libethash-cuda/CUDAMiner.h>
#endif

boost::asio::io_service g_io_service;
bool g_exitOnError = false;  // Whether or not progminer should exit on mining threads errors

static std::thread m_io_thread;

using namespace dev::eth;

struct Settings {
  std::map<std::string, DeviceDescriptor> devices;
  CUSettings cuSettings;
  CLSettings clSettings;
};

extern "C" {

void* progminer_setup_devices() {
  Settings* settings = new Settings;

#if ETH_ETHASHCL
  CLMiner::enumDevices(settings->devices);
#endif
#if ETH_ETHASHCUDA
  CUDAMiner::enumDevices(settings->devices);
#endif
  // Subscribe devices with appropriate Miner Type
  // Use CUDA first when available then, as second, OpenCL

  // Apply discrete subscriptions (if any)
#if ETH_ETHASHCUDA
  if (settings->cuSettings.devices.size())
  {
    for (auto index : settings->cuSettings.devices)
    {
      if (index < settings->devices.size())
      {
        auto it = settings->devices.begin();
        std::advance(it, index);
        if (!it->second.cuDetected)
          throw std::runtime_error("Can't CUDA subscribe a non-CUDA device.");
        it->second.subscriptionType = DeviceSubscriptionTypeEnum::Cuda;
      }
    }
  }
#endif
#if ETH_ETHASHCL
  if (settings->clSettings.devices.size())
  {
    for (auto index : settings->clSettings.devices)
    {
      if (index < settings->devices.size())
      {
        auto it = settings->devices.begin();
        std::advance(it, index);
        if (!it->second.clDetected)
          throw std::runtime_error("Can't OpenCL subscribe a non-OpenCL device.");
        if (it->second.subscriptionType != DeviceSubscriptionTypeEnum::None)
          throw std::runtime_error(
            "Can't OpenCL subscribe a CUDA subscribed device.");
        it->second.subscriptionType = DeviceSubscriptionTypeEnum::OpenCL;
      }
    }
  }
#endif

  // Subscribe all detected devices
#if ETH_ETHASHCUDA
  if (!settings->cuSettings.devices.size())
  {
    for (auto it = settings->devices.begin(); it != settings->devices.end(); it++)
    {
      if (!it->second.cuDetected ||
        it->second.subscriptionType != DeviceSubscriptionTypeEnum::None)
        continue;
      it->second.subscriptionType = DeviceSubscriptionTypeEnum::Cuda;
    }
  }
#endif
#if ETH_ETHASHCL
  if (!settings->clSettings.devices.size())
  {
    for (auto it = settings->devices.begin(); it != settings->devices.end(); it++)
    {
      if (!it->second.clDetected ||
        it->second.subscriptionType != DeviceSubscriptionTypeEnum::None)
        continue;
      it->second.subscriptionType = DeviceSubscriptionTypeEnum::OpenCL;
    }
  }
#endif
  // Count of subscribed devices
  int subscribedDevices = 0;
  for (auto it = settings->devices.begin(); it != settings->devices.end(); it++)
  {
    if (it->second.subscriptionType != DeviceSubscriptionTypeEnum::None)
      subscribedDevices++;
  }

  if (!subscribedDevices) {
    return NULL;
  }

  return settings;
}

void progminer_start(Settings* settings, void (*on_solution_found)(uint32_t nonce, int height)) {
  boost::asio::io_context::work work(g_io_service);
  // Start io_service in it's own thread
  m_io_thread = std::thread{boost::bind(&boost::asio::io_service::run, &g_io_service)};

  FarmSettings farmSettings;
  CPSettings cpSettings;
  Farm* f = new Farm(settings->devices, farmSettings, settings->cuSettings, settings->clSettings, cpSettings);

  f->onSolutionFound([on_solution_found](const Solution& sol) {
    auto r = EthashAux::eval(sol.work.epoch, sol.work.block, sol.work.header, sol.nonce);
    bool accepted = r.value <= sol.work.boundary;
    if (accepted) {
      on_solution_found(sol.nonce, sol.work.block);
    }
  });
  f->start();
}

extern "C" void sha256d(unsigned char *hash, const unsigned char *data, int len);

void progminer_set_work(uint32_t header[32], int height, uint8_t* target) {
  uint8_t header_hash[32];
  uint32_t nonce = header[19];
  header[19] = 0;
  sha256d(header_hash, (unsigned char*)header, 80);
  header[19] = nonce;
  WorkPackage wp;
  wp.header = dev::h256(header_hash, dev::h256::ConstructFromPointer);
  wp.block = height;
  for (int i=0; i<32; ++i) {
    wp.boundary.data()[31-i] = target[i];
  }
  wp.epoch = ethash::get_epoch_number(height);
  if (Farm::f().paused()) {
    Farm::f().resume();
  }
  Farm::f().setWork(wp);
}

void progminer_pause() {
  Farm::f().pause();
}

}
