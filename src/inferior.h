#include <sys/types.h>
#include <unistd.h>

enum InferiorState {
  INFERIOR_RUNNING,
  INFERIOR_STOPPED,
};

struct Inferior {
  pid_t pid;
  enum InferiorState state;
};
