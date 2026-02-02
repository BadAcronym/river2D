//TEST: verify if time outputs correctly
void river2D_queryTime
(
    River2D_Time *time
){
    LARGE_INTEGER t1;

    KeQuerySystemTimePrecise(&time);

    time->s = t1.QuadPart / 10000000;
    time->ns = t1.QuadPart * 100;
}
