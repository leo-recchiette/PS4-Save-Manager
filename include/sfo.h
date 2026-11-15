#ifndef _SFO_H_
#define _SFO_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SFO_ACCOUNT_ID_SIZE 8
#define SFO_PSID_SIZE 16

typedef struct sfo_context_s sfo_context_t;

typedef struct {
	u32 flags;
	u32 user_id;
	u64 account_id;
	u8* psid;
} sfo_patch_t;

sfo_context_t * sfo_alloc(void);
void sfo_free(sfo_context_t *context);

int sfo_read(sfo_context_t *context, const char *file_path);
int sfo_write(sfo_context_t *context, const char *file_path);


u8* sfo_get_param_value(sfo_context_t *in, const char* param);

int patch_sfo(const char *in_file_path, sfo_patch_t* patches);

#ifdef __cplusplus
}
#endif

#endif /* !_SFO_H_ */
