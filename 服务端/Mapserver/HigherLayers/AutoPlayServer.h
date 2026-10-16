#ifndef DRAGON_AUTOPLAY_SERVER_H
#define DRAGON_AUTOPLAY_SERVER_H

// Payload is copied only after its exact wire size has been validated.
void RecvAutoPlayQuery(int connection, const void* payload, int payload_size);

// Applied to all player casts, independently of whether queries were used.
void ResetAutoPlaySession(int connection);
void BeginPlayerMagicPreparation(int connection, unsigned char skill,
    unsigned short ready_ms);
bool CanExecutePlayerMagic(int connection, unsigned char skill);

#endif
