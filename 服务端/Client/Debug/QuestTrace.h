#pragma once
#include <cstddef>
void QuestTraceShow();
void QuestTraceTick();
void QuestTraceReceive(const void* data, std::size_t size);
void QuestTraceCancelNavigation();
void QuestTraceReset();
