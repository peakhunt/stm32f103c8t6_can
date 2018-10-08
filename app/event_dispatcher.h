#ifndef EVENT_DISPATCHER_H_
#define EVENT_DISPATCHER_H_

typedef void (*event_handler)(uint32_t event);

extern void event_dispatcher_init(void);
extern void event_set(uint32_t bits);
extern void event_register_handler(event_handler handler, uint32_t event);
extern void event_dispatcher_dispatch(void);

#endif /* EVENT_DISPATCHER_H_ */
