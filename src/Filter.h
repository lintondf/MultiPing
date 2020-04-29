#ifndef _FILTER_H
#define _FILTER_H 1

class Filter {
    public:
        Filter( float alpha, float max ) : a(alpha), oma(1.0f-alpha), n(0), last(max) {
            for (unsigned int i = 0; i < N; i++) {
                history[N] = max;
            }
        }

        float add( float v ) {
            //history[n++ % N] = v;
            float mv = v; /*median(history[0], history[1], history[2], history[3], history[4] );*/
            last = oma * last + a * mv;
            return last;
        }

    protected:
        float last;
        float a;
        float oma;  // 1-a
        static const unsigned int N = 5;
        float history[N];
        unsigned int n;

        float median(float a, float b, float c, float d, float e)
        {
        #define exchange(a,b) {float t = b; b = a; a = t;}
        #define order(a,b) if(a>b){ exchange(a,b); }
            order(a,b);
            order(d,e);  
            order(a,c);
            order(b,c);
            order(a,d);  
            order(c,d);
            order(b,e);
            order(b,c);
        #undef exchange
        #undef order
            return c;
        }        
};

#endif // _FILTER_H
