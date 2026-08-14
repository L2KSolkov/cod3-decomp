// ============================================================================
// tr_qsort.cpp - render.o draw-surf quicksort (tr_main.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include <stdint.h>

// drawSurf_s (IDA type; size 8)
enum surfaceType_t;
struct drawSurf_s {
    unsigned int sort;      // +0x00
    surfaceType_t* surface; // +0x04
};

// ============================================================================
// shortsort (file-static) - ea: 0x006C1CD0
// ============================================================================
static void shortsort(drawSurf_s* lo, drawSurf_s* hi)
{
    for (; hi > lo; --hi)
    {
        drawSurf_s* i = lo;
        for (drawSurf_s* v2 = lo + 1; v2 <= hi; ++v2)
        {
            if (v2->sort > i->sort)
                i = v2;
        }
        unsigned int sort = i->sort;
        i->sort = hi->sort;
        surfaceType_t* surface = hi->surface;
        hi->sort = sort;
        surfaceType_t* v6 = i->surface;
        i->surface = surface;
        hi->surface = v6;
    }
}

// ============================================================================
// qsortFast - ea: 0x006C1D20
// ============================================================================
void qsortFast(void* basePtr, unsigned int num, unsigned int width)
{
    if (num >= 2 && width != 0)
    {
        drawSurf_s* base = (drawSurf_s*)basePtr;
        unsigned int v3 = width;
        unsigned int v4 = width * (num - 1);
        char* histk[30];
        char* lostk[30];
        int temp;
        int stkptr = 0;
        drawSurf_s* v5 = base;
        drawSurf_s* v6 = (drawSurf_s*)((char*)base + v4);

        while (1)
        {
            int* v14;
            drawSurf_s* i;
            while (1)
            {
                unsigned int v7 = ((char*)v6 - (char*)v5) / v3 + 1;
                if (v7 <= 8)
                {
                    shortsort(v5, v6);
                    goto label6;
                }
                int v9 = (int)(v3 * (v7 >> 1));
                unsigned int* v11 = (unsigned int*)((char*)&v5->sort + v9);
                unsigned int v10 = *v11;
                *v11 = v5->sort;
                surfaceType_t* surface = v5->surface;
                v5->sort = v10;
                surfaceType_t* v13 = (surfaceType_t*)v11[1];
                v11[1] = (unsigned int)surface;
                v5->surface = v13;

                v14 = (int*)v5;
                i = (drawSurf_s*)((char*)v6 + v3);
                while (1)
                {
                    do
                    {
                        v14 = (int*)((char*)v14 + v3);
                    } while ((char*)v14 <= (char*)v6 && *(unsigned int*)v14 <= v5->sort);
                    do
                    {
                        i = (drawSurf_s*)((char*)i - v3);
                    } while (i > v5 && i->sort >= v5->sort);
                    if (i < (drawSurf_s*)v14)
                        break;
                    temp = *(int*)v14;
                    *(int*)v14 = (int)i->sort;
                    i->sort = (unsigned int)temp;
                    temp = v14[1];
                    v14[1] = (int)i->surface;
                    i->surface = (surfaceType_t*)temp;
                }

                unsigned int sort = v5->sort;
                v5->sort = i->sort;
                surfaceType_t* v17 = i->surface;
                i->sort = sort;
                surfaceType_t* v18 = v5->surface;
                v5->surface = v17;
                i->surface = v18;

                if ((char*)i - (char*)v5 - 1 >= (char*)v6 - (char*)v14)
                    break;
                if ((char*)v14 < (char*)v6)
                {
                    lostk[stkptr] = (char*)v14;
                    histk[stkptr++] = (char*)v6;
                }
                v3 = width;
                if ((char*)v5 + width >= (char*)i)
                {
label6:
                    int v8 = --stkptr;
                    if (stkptr < 0)
                        return;
                    v5 = (drawSurf_s*)lostk[v8];
                    v6 = (drawSurf_s*)histk[v8];
                }
                else
                {
                    v6 = (drawSurf_s*)((char*)i - width);
                }
            }
            v3 = width;
            if ((char*)v5 + width < (char*)i)
            {
                lostk[stkptr] = (char*)v5;
                histk[stkptr++] = (char*)i - width;
            }
            if ((char*)v14 >= (char*)v6)
                goto label6;
            v5 = (drawSurf_s*)v14;
        }
    }
}
