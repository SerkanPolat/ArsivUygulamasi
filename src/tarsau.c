#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
int IzinleriYonet(struct stat fileStat){
	int izin = 0;
	int AraToplam = 0;
    izin += S_ISDIR(fileStat.st_mode)*1000;
    AraToplam += (fileStat.st_mode & S_IRUSR)/64;
    AraToplam += (fileStat.st_mode & S_IWUSR)/64; 
    AraToplam += (fileStat.st_mode & S_IXUSR)/64;
	izin += AraToplam*100;
	AraToplam = 0;
    AraToplam += (fileStat.st_mode & S_IRGRP)/8;
    AraToplam += (fileStat.st_mode & S_IWGRP)/8;
    AraToplam += (fileStat.st_mode & S_IXGRP)/8;
	izin += AraToplam*10;
	AraToplam = 0;
    AraToplam += (fileStat.st_mode & S_IROTH); 
    AraToplam += (fileStat.st_mode & S_IWOTH); 
    AraToplam += (fileStat.st_mode & S_IXOTH); 
	 izin +=AraToplam;
	return izin;
}

void ArsivOlustur(int argc,char** argv){

	struct stat fileStat;
	int Izinler;
	int ArsivAdiMevcut = 0;
	char ArsivAdi[20] = "a.sau";
	if(!strcmp(argv[2],"-o")){
		printf("Parametre Hatasi\n");
		return ;
	}

	for(int i = 3;i<argc;i++){
		if(!strcmp(argv[i],"-o")){
			if(i+1!=argc){
				
				ArsivAdiMevcut = 2;
				strcpy(ArsivAdi,argv[i+1]);
			}else{
				printf("Arsiv Adi Girilmedi.(Parametre Hatasi)\n");
				return;
			}
		}
	}
	int boyutKontrol=0;
	for(int i = 2 ;i<argc-ArsivAdiMevcut;i++){
		if(stat(argv[i],&fileStat) < 0)
       		printf("\n%s dosya bulunamadi\n",argv[i]);
		else{
			boyutKontrol += fileStat.st_size;
		}
	}
	if(argc-ArsivAdiMevcut>32){ //32 Dosya Kontrolu
		printf("Maksimum 32 Dosya Girilebilir.");
		return ;
	}
	if(boyutKontrol>209715200){ //200 MB Boyut Kontrolu
		printf("Girilen Dosyalarin Maksimum Boyutu 200 MB Asmaktadir.");
		return ;
	}
	FILE *binaryDosyaKontrolu; //Binary Dosya Kontrolu Ascii Uzerinden
	for(int i = 2 ;i<argc-ArsivAdiMevcut;i++){
		binaryDosyaKontrolu = fopen(argv[i],"r");	
		if(stat(argv[i],&fileStat)>=0){
			if(fileStat.st_size<50){
				for(int j = 0;j<fileStat.st_size-2;j++){
					
					if(fgetc(binaryDosyaKontrolu)<5){
						printf("%s Dosya Bicimi Uyumsuz",argv[i]);
						return;	
					}
				}
			}else{
				for(int j = 0 ;j<40;j++){	
					if(fgetc(binaryDosyaKontrolu)<5){
						printf("%s Dosya Bicimi Uyumsuz\n",argv[i]);
						return;	
					}
				}
			}
			fclose(binaryDosyaKontrolu);
		}
	}
	FILE *dosya = fopen(ArsivAdi,"w");
	fprintf(dosya,"0000000000");
	for(int i = 2 ;i<argc-ArsivAdiMevcut;i++){
		if(stat(argv[i],&fileStat) < 0)
       			printf("\n%s Dosya Bulunamadi\n",argv[i]);
		else{
			Izinler = IzinleriYonet(fileStat);
			fprintf(dosya,"|%s ,%d,%d",argv[i],Izinler,fileStat.st_size);
		}
	}
	fputc('|',dosya);
	int IlkBolumSize = ftell(dosya);
	int Boyut[10] ={0,0,0,0,0,0,0,0,0,0};
	int BasamakSayisi=9;
	for(int i = 10;i<1000000000;i=i*10){
		if(IlkBolumSize/i>0){
			if(i==10){
			Boyut[BasamakSayisi]=IlkBolumSize%i;
			BasamakSayisi--;
			}
			Boyut[BasamakSayisi]=IlkBolumSize/i;
			BasamakSayisi--;
		}
		
	}
	fseek(dosya,0,SEEK_SET);
	for(int i = 0;i<10;i++){
		fputc((char)(Boyut[i]+48),dosya);
	}
	fseek(dosya,IlkBolumSize,SEEK_SET);
	char okunanKarakter;
	for(int i=2;i<argc-ArsivAdiMevcut;i++){
		FILE *dosya2 = fopen(argv[i],"r");
		okunanKarakter = fgetc(dosya2);
		while(okunanKarakter!=EOF){
			fputc(okunanKarakter,dosya);
			okunanKarakter=fgetc(dosya2);
		}
		fclose(dosya2);
	}
	fclose(dosya);
	printf("Arsiv Olusturuldu.");
	return;
}
struct DosyaYapisi{
	char dosyaAdi[100];
	int dosyaBoyutu;
	int dosyaYetkileri;
	struct DosyaYapisi *next;
};

int sekizlikOnlukDonusum(int DonusecekSayi){ //Dosyadan Okunan Yetkiler open fonksiyonunun Mode bolumunde 8lik 10luk Taban Sorununa Ugruyor.
	int i = 0;
	int OnlukSayi=0;
	while (DonusecekSayi != 0)
    {
        OnlukSayi =  OnlukSayi +(DonusecekSayi % 10)* pow(8, i++);
        DonusecekSayi = DonusecekSayi / 10;
    }
	return OnlukSayi;
}

struct DosyaYapisi* birinciBolumDosyaBilgileriOku(int argc,char** argv,FILE *dosya){
		struct DosyaYapisi *ilkDosya = malloc(sizeof(struct DosyaYapisi));
		int birinciBolumBoyut;
		fscanf(dosya,"%d",&birinciBolumBoyut);
		fscanf(dosya,"|%s ,%d,%d",ilkDosya->dosyaAdi,&ilkDosya->dosyaYetkileri,&ilkDosya->dosyaBoyutu);			
		ilkDosya->next = malloc(sizeof(struct DosyaYapisi));
		struct DosyaYapisi *iter = ilkDosya;
		ilkDosya->next = NULL;
		while(ftell(dosya)+1<birinciBolumBoyut){
			iter->next = malloc(sizeof(struct DosyaYapisi));
			iter = iter->next;
			fscanf(dosya,"|%s ,%d,%d",iter->dosyaAdi,&iter->dosyaYetkileri,&iter->dosyaBoyutu);
			iter->next = NULL;
		}
	return ilkDosya;
} 

void ArsivCikart(int argc,char** argv){
	
	int dosyaTanimlayicisi;
	FILE *dosya = fopen(argv[2],"r");

	
	if(argc==4){
	//Arsiv Belirtilen Konuma Acilacak

			struct DosyaYapisi *iter = birinciBolumDosyaBilgileriOku(argc,argv,dosya);
			
			fseek(dosya,1,SEEK_CUR); //Ikinci Bolumun Baslangici			
			char Dizin[150];		
			mkdir(argv[3],0700);
			strcpy(Dizin,argv[3]);
			strcat(Dizin,"/");
			char *geciciBellek;
			int i;
			while(iter != NULL){
			strcpy(geciciBellek,Dizin);
			char *uzantilidosyaAd = strcat(geciciBellek,iter->dosyaAdi);
			dosyaTanimlayicisi = open(uzantilidosyaAd,O_CREAT | O_WRONLY,sekizlikOnlukDonusum(iter->dosyaYetkileri));	
			for(i=0;i<iter->dosyaBoyutu;i++){
				char karakter = fgetc(dosya);
				write(dosyaTanimlayicisi,&karakter,1);
			}
			close(dosyaTanimlayicisi);
			iter = iter->next;		
		}
		fclose(dosya);
	}else if(argc==3){
	//Arsiv Dosyanin Bulundugu Dizine Acilacak
		struct DosyaYapisi *iter = birinciBolumDosyaBilgileriOku(argc,argv,dosya);
		struct DosyaYapisi *temizleyici = iter;
		fseek(dosya,1,SEEK_CUR); //Ikinci Bolumun Baslangici
		
		int i;
		while(iter != NULL){
			dosyaTanimlayicisi = open(iter->dosyaAdi,O_CREAT | O_WRONLY,sekizlikOnlukDonusum(iter->dosyaYetkileri));	
			for(i=0;i<iter->dosyaBoyutu;i++){
				char karakter = fgetc(dosya);
				write(dosyaTanimlayicisi,&karakter,1);
			}
			close(dosyaTanimlayicisi);
			iter = iter->next;		
		}
		fclose(dosya);
		iter = temizleyici;		
		while(iter!=NULL){
			printf("%s ",iter->dosyaAdi);
			temizleyici = iter;
			iter = iter->next;
			free(temizleyici);
		}
		printf("Dosyalari mevcut Dizine Acildi\n");
				
	}else{
		printf("\nParametre Sayisi Uyumsuz\n");
	}
	return;
}

int main(int argc,char** argv){

	if(!strcmp(argv[1],"-b")){
		ArsivOlustur(argc,argv);
		return 1;
	}else if(!strcmp(argv[1],"-a")){
		if(argc>2)		
			ArsivCikart(argc,argv);	
			
	}
	return 0;
}
