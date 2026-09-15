gcc -Wall -Wextra -std=c11 -I include     src/models/latency_model.c src/models/packet_loss_model.c     src/models/jitter_model.c src/models/bandwidth_model.c     tests/test_models.c -o test_models -lm

./test_models