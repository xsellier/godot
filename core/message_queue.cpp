/*************************************************************************/
/*  message_queue.cpp                                                    */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "message_queue.h"
#include "globals.h"
#include "script_language.h"
MessageQueue *MessageQueue::singleton = NULL;

MessageQueue *MessageQueue::get_singleton() {

	return singleton;
}

Error MessageQueue::push_call(ObjectID p_id, const StringName &p_method, const Variant **p_args, int p_argcount, bool p_show_error) {

	_THREAD_SAFE_METHOD_

	int room_needed = sizeof(Message) + sizeof(Variant) * p_argcount;

	if ((buffer_end + room_needed) >= buffer_size) {
		String type;
		if (ObjectDB::get_instance(p_id))
			type = ObjectDB::get_instance(p_id)->get_type();
		ERR_PRINTS("failed method: " + type + ":" + p_method + " target ID: " + itos(p_id));
		statistics();
		ERR_EXPLAIN("Message queue out of memory. Try increasing 'message_queue_size_kb' in project settings");
		ERR_FAIL_V(ERR_OUT_OF_MEMORY);
	}

	Message *msg = memnew_placement(&buffer[buffer_end], Message);
	msg->args = p_argcount;
	msg->instance_ID = p_id;
	msg->target = p_method;
	msg->type = TYPE_CALL;
	if (p_show_error)
		msg->type |= FLAG_SHOW_ERROR;

	buffer_end += sizeof(Message);

	for (int i = 0; i < p_argcount; i++) {

		Variant *v = memnew_placement(&buffer[buffer_end], Variant);
		buffer_end += sizeof(Variant);
		*v = *p_args[i];
	}

	return OK;
}

Error MessageQueue::push_call(ObjectID p_id, const StringName &p_method, VARIANT_ARG_DECLARE) {

	VARIANT_ARGPTRS;

	int argc = 0;

	for (int i = 0; i < VARIANT_ARG_MAX; i++) {
		if (argptr[i]->get_type() == Variant::NIL)
			break;
		argc++;
	}

	return push_call(p_id, p_method, argptr, argc, false);
}

Error MessageQueue::push_set(ObjectID p_id, const StringName &p_prop, const Variant &p_value) {

	_THREAD_SAFE_METHOD_

	uint8_t room_needed = sizeof(Message) + sizeof(Variant);

	if ((buffer_end + room_needed) >= buffer_size) {
		String type;
		if (ObjectDB::get_instance(p_id))
			type = ObjectDB::get_instance(p_id)->get_type();
		WARN_PRINTS("failed set: " + type + ":" + p_prop + " target ID: " + itos(p_id));
		statistics();
		ERR_EXPLAIN("Message queue out of memory. Try increasing 'message_queue_size_kb' in project settings");
		ERR_FAIL_V(ERR_OUT_OF_MEMORY);
	}

	Message *msg = memnew_placement(&buffer[buffer_end], Message);
	msg->args = 1;
	msg->instance_ID = p_id;
	msg->target = p_prop;
	msg->type = TYPE_SET;

	buffer_end += sizeof(Message);

	Variant *v = memnew_placement(&buffer[buffer_end], Variant);
	buffer_end += sizeof(Variant);
	*v = p_value;

	return OK;
}

Error MessageQueue::push_notification(ObjectID p_id, int p_notification) {

	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(p_notification < 0, ERR_INVALID_PARAMETER);

	uint8_t room_needed = sizeof(Message);

	if ((buffer_end + room_needed) >= buffer_size) {
		String type;
		if (ObjectDB::get_instance(p_id))
			type = ObjectDB::get_instance(p_id)->get_type();
		ERR_PRINTS("failed notification: " + itos(p_notification) + " target ID: " + itos(p_id));
		statistics();
		ERR_EXPLAIN("Message queue out of memory. Try increasing 'message_queue_size_kb' in project settings");
		ERR_FAIL_V(ERR_OUT_OF_MEMORY);
	}

	Message *msg = memnew_placement(&buffer[buffer_end], Message);

	msg->type = TYPE_NOTIFICATION;
	msg->instance_ID = p_id;
	//msg->target;
	msg->notification = p_notification;

	buffer_end += sizeof(Message);

	return OK;
}

Error MessageQueue::push_call(Object *p_object, const StringName &p_method, VARIANT_ARG_DECLARE) {

	return push_call(p_object->get_instance_ID(), p_method, VARIANT_ARG_PASS);
}

Error MessageQueue::push_notification(Object *p_object, int p_notification) {

	return push_notification(p_object->get_instance_ID(), p_notification);
}
Error MessageQueue::push_set(Object *p_object, const StringName &p_prop, const Variant &p_value) {

	return push_set(p_object->get_instance_ID(), p_prop, p_value);
}

void MessageQueue::statistics() {

	Map<StringName, int> set_count;
	Map<int, int> notify_count;
	Map<StringName, int> call_count;
	int null_count = 0;

	uint32_t read_pos = 0;
	while (read_pos < buffer_end) {
		Message *message = (Message *)&buffer[read_pos];

		Object *target = ObjectDB::get_instance(message->instance_ID);

		if (target != NULL) {

			switch (message->type & FLAG_MASK) {

				case TYPE_CALL: {

					if (!call_count.has(message->target))
						call_count[message->target] = 0;

					call_count[message->target]++;

				} break;
				case TYPE_NOTIFICATION: {

					if (!notify_count.has(message->notification))
						notify_count[message->notification] = 0;

					notify_count[message->notification]++;

				} break;
				case TYPE_SET: {

					if (!set_count.has(message->target))
						set_count[message->target] = 0;

					set_count[message->target]++;

				} break;
			}

		} else {

			null_count++;
		}

		read_pos += sizeof(Message);
		if ((message->type & FLAG_MASK) != TYPE_NOTIFICATION)
			read_pos += sizeof(Variant) * message->args;
	}

	ERR_PRINTS("TOTAL BYTES: " + itos(buffer_end));
	ERR_PRINTS("NULL count: " + itos(null_count));

	for (Map<StringName, int>::Element *E = set_count.front(); E; E = E->next()) {

		ERR_PRINTS("SET " + E->key() + ": " + itos(E->get()));
	}

	for (Map<StringName, int>::Element *E = call_count.front(); E; E = E->next()) {

		ERR_PRINTS("CALL " + E->key() + ": " + itos(E->get()));
	}

	for (Map<int, int>::Element *E = notify_count.front(); E; E = E->next()) {

		ERR_PRINTS("NOTIFY " + itos(E->key()) + ": " + itos(E->get()));
	}
}

bool MessageQueue::print() {

	return false;
}

int MessageQueue::get_max_buffer_usage() const {

	return buffer_max_used;
}

void MessageQueue::_call_function(Object *p_target, const StringName &p_func, const Variant *p_args, int p_argcount, bool p_show_error) {

	const Variant **argptrs = NULL;
	if (p_argcount) {
		argptrs = (const Variant **)alloca(sizeof(Variant *) * p_argcount);
		for (int i = 0; i < p_argcount; i++) {
			argptrs[i] = &p_args[i];
		}
	}

	Variant::CallError ce;
	p_target->call(p_func, argptrs, p_argcount, ce);
	if (p_show_error && ce.error != Variant::CallError::CALL_OK) {

		ERR_PRINTS("Error calling deferred method: " + Variant::get_call_error_text(p_target, p_func, argptrs, p_argcount, ce));
	}
}

void MessageQueue::flush() {

	if (buffer_end > buffer_max_used) {
		buffer_max_used = buffer_end;
	}

	uint32_t read_pos = 0;

	//using reverse locking strategy
	_THREAD_SAFE_LOCK_
	ERR_FAIL_COND(flushing);
	flushing = true;

	while (read_pos < buffer_end) {

		//lock on each interation, so a call can re-add itself to the message queue

		Message *message = (Message *)&buffer[read_pos];

		uint32_t advance = sizeof(Message);
		if ((message->type & FLAG_MASK) != TYPE_NOTIFICATION)
			advance += sizeof(Variant) * message->args;

		//pre-advance so this function is reentrant
		read_pos += advance;

		_THREAD_SAFE_UNLOCK_

		Object *target = ObjectDB::get_instance(message->instance_ID);

		if (target != NULL) {

			switch (message->type & FLAG_MASK) {
				case TYPE_CALL: {

					Variant *args = (Variant *)(message + 1);

					// messages don't expect a return value

					_call_function(target, message->target, args, message->args, message->type & FLAG_SHOW_ERROR);

				} break;
				case TYPE_NOTIFICATION: {

					// messages don't expect a return value
					target->notification(message->notification);

				} break;
				case TYPE_SET: {

					Variant *arg = (Variant *)(message + 1);
					// messages don't expect a return value
					target->set(message->target, *arg);
				} break;
			}
		}

		if ((message->type & FLAG_MASK) != TYPE_NOTIFICATION) {
			Variant *args = (Variant *)(message + 1);
			for (int i = 0; i < message->args; i++) {
				args[i].~Variant();
			}
		}

		message->~Message();

		_THREAD_SAFE_LOCK_
	}

	buffer_end = 0; // reset buffer
	flushing = false;
	_THREAD_SAFE_UNLOCK_
}

bool MessageQueue::is_flushing() const {

	return flushing;
}

MessageQueue::MessageQueue() {

	ERR_FAIL_COND(singleton != NULL);
	singleton = this;
	flushing = false;

	buffer_end = 0;
	buffer_max_used = 0;
	buffer_size = GLOBAL_DEF("core/message_queue_size_kb", DEFAULT_QUEUE_SIZE_KB);
	buffer_size *= 1024;
	buffer = memnew_arr(uint8_t, buffer_size);
}

MessageQueue::~MessageQueue() {

	uint32_t read_pos = 0;

	while (read_pos < buffer_end) {

		Message *message = (Message *)&buffer[read_pos];
		Variant *args = (Variant *)(message + 1);
		int argc = message->args;
		if ((message->type & FLAG_MASK) != TYPE_NOTIFICATION) {
			for (int i = 0; i < argc; i++)
				args[i].~Variant();
		}
		message->~Message();

		read_pos += sizeof(Message);
		if ((message->type & FLAG_MASK) != TYPE_NOTIFICATION)
			read_pos += sizeof(Variant) * message->args;
	}

	singleton = NULL;
	memdelete_arr(buffer);
}
