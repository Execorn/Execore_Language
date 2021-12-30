//
// Created by Legion on 07.12.2021.
//

#include "src/handle_err.h"
#include "null_target.h"


static TargetNAN default_nan = {
        .usages         = 0,
        .type           = NAN_T,
        .target_type    = (TargetType *) &NanT,
};


static TargetNAN* ConstructNan() {return &default_nan;}

static void FreeNan() {}

static void PrintNan() {printf("NOTHING");}

static void InitNan() {}

static void NanAdv_init() {}


static TargetNAN* GetNanMethod(Target* target, char *name) {
    RaiseError(SYNTAX_ERROR, "whe the fuck did u find %s %s?", TARGET_NAME(target), name);

	return ConstructNan();
}


/*	None target_id API.
 */
Nan_T NanT = {
	.name = "default_nan",
	.construct = (Target* (*)()) ConstructNan,
	.free = (void (*)(Target* )) FreeNan,
	.dump = (void (*)(FILE*, Target*)) PrintNan,
	.init = (void (*)()) InitNan,
	.adv_init = (void (*)()) NanAdv_init,
	.method = (Target* (*)()) GetNanMethod
};
