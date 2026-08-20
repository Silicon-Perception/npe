#ifndef NPP_H
#define NPP_H

#include "npp_types.h"
#include "npp_schema.h"
#include "npp_frame.h"
#include "npp_backend.h"
#include "npp_crypto.h"
#include "npp_transport.h"
#include "npp_session.h"
#include "npp_server.h"

/* NPP 2.0 modules */
#include "npp_frame2.h"
#include "npp_hopping.h"
#include "npp_reservoir.h"
#include "npp_validation.h"
#include "npp_subscription.h"
#include "npp_extension.h"
#include "npp_coloring.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NPP_VERSION_MAJOR 2
#define NPP_VERSION_MINOR 0
#define NPP_VERSION_PATCH 0
#define NPP_VERSION_STRING "2.0.0"

const char* npp_version_string(void);

#ifdef __cplusplus
}
#endif

#endif