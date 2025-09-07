#include "Grove_lib.h"
#include "Cube_general.h"

int main()
{
    bool cancelled;

    SSCMA AI;

    stdio_init_all();

    AI.begin();
    cube_init();

    while(1) {
        if (!AI.invoke()) {
            printf("invoke success\n");
            printf("perf: prepocess=%u, inference=%u, postprocess=%u\n",
                (unsigned)AI.perf().prepocess,
                (unsigned)AI.perf().inference,
                (unsigned)AI.perf().postprocess);

            for (int i = 0; i < AI.boxes().size(); i++) {
                printf("Box[%d] target=%u, score=%u, x=%u, y=%u, w=%u, h=%u\n",
                        i,
                        (unsigned)AI.boxes()[i].target,
                        (unsigned)AI.boxes()[i].score,
                        (unsigned)AI.boxes()[i].x,
                        (unsigned)AI.boxes()[i].y,
                        (unsigned)AI.boxes()[i].w,
                        (unsigned)AI.boxes()[i].h);
            }

            for (int i = 0; i < AI.classes().size(); i++) {
                printf("Class[%d] target=%u, score=%u\n",
                        i,
                        (unsigned)AI.classes()[i].target,
                        (unsigned)AI.classes()[i].score);
            }

            for (int i = 0; i < AI.points().size(); i++) {
                printf("Point[%d]: target=%u, score=%u, x=%u, y=%u\n",
                    i,
                    (unsigned)AI.points()[i].target,
                    (unsigned)AI.points()[i].score,
                    (unsigned)AI.points()[i].x,
                    (unsigned)AI.points()[i].y);
            }
                if (AI.boxes()[0].target == 0) {
                    add_repeating_timer_ms(16, timer_callback_rock, NULL, &timer_rock);
                    sleep_ms(5000);
                    cancelled = cancel_repeating_timer(&timer_rock);
                }
                
                else if (AI.boxes()[0].target == 1) {
                    add_repeating_timer_ms(16, timer_callback_paper, NULL, &timer_paper);
                    sleep_ms(5000);
                    cancelled = cancel_repeating_timer(&timer_paper);
                }

                else if (AI.boxes()[0].target == 2) {
                    add_repeating_timer_ms(16, timer_callback_scissors, NULL, &timer_scissors);
                    sleep_ms(5000);
                    cancelled = cancel_repeating_timer(&timer_scissors);
                }
        }
    }
}