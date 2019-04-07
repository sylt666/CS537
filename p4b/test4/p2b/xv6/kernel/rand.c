#define M 2147483647
#define A 16807
#define Q ( M / A )
#define R ( M % A )

static int seed = 1;

int getrand(int max)
{
    seed = A * (seed % Q) - R * (seed / Q);

    if (seed <= 0)
    {
        seed += M;
    }

    return (seed % max + 1);
}
