//µİ¹éÇóºÍ
int sum(int *a,int n)  
{   
    return n==0?0:sum(a,n-1)+a[n-1];  
}