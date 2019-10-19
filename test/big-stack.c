#define W 1000
#define H 1000
#define MAX 100000 
void initImg(int img[], float dtr[])
{
    for(int i=0;i<W;i++)
        for(int j=0;j<H;j++)
            img[i*W+j]=255;

    for(int j=0;j<H;j++)
    {
        img[j] = 0;
        img[W*(W-1)+j] = 0;
    }
    for(int i=0;i<W;i++)
    {
        img[i*W] = 0;
        img[i*W+H-1] = 0;
    }
    for(int i=0;i<W;i++)
        for(int j=0;j<H;j++)
        { 
            if(img[i*W+j]==0)
                dtr[i*W+j] = 0;    // <------here
            else
                dtr[i*W+j] = MAX;  // <------here
        }
}
int main()
{
    int image[W*H];
    float dtr[W*H];
    initImg(image,dtr);
    return 0;
}
