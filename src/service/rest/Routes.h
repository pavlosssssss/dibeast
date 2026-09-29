#pragma once

#include "application/AppDispatcher.h"
#include "application/CorsPolicy.h"
#include "application/DeviceController.h"
#include "service/rest/Router.h"

namespace dibeast::service {

void registerRoutes(Router& router, const app::DeviceController& device, app::AppDispatcher& apps,
                    const app::CorsPolicy& cors);

}  // namespace dibeast::service
