#ifndef EVENT_TYPES_H_
#define EVENT_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t EventType;

typedef union {
    uint32_t value;
    uint8_t  val_array[4];
    void*    ptr;
} un_event_param;

typedef struct
{
    uint8_t         type;
    un_event_param  param;
} Event;

typedef void (*EventHandler)(const Event* e);

typedef void (*Subs_EventHandler)(const Event *e);

typedef struct {
    uint8_t event_type;
    Subs_EventHandler handler;
    uint8_t slot_id;
} st_common_event_subs_params;

#ifdef __cplusplus
}
#endif

#endif /* EVENT_TYPES_H_ */
