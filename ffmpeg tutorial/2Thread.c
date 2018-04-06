// For SDL 2   
// tutorial02.c  
// A pedagogical video player that will stream through every video frame as fast as it can.  
//  
// This tutorial was written by Stephen Dranger (dranger@gmail.com).  
//  
// Code based on FFplay, Copyright (c) 2003 Fabrice Bellard,  
// and a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)  
// Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1  
//  
// Use the Makefile to build all examples.  
//  
// Run using  
// tutorial02 myvideofile.mpg  
//  
// to play the video stream on your screen.  
#include <libavcodec/avcodec.h>  
#include <libavformat/avformat.h>  
#include <libswscale/swscale.h>  
  
#include <SDL.h>  
#include <SDL_thread.h>  
  
#ifdef __MINGW32__  
#undef main /* Prevents SDL from overriding main() */  
#endif  
  
#include <stdio.h>  

#include <stdlib.h>
#include <pthread.h>

  
int  
randomInt(int min, int max)  
{  
    return min + rand() % (max - min + 1);  
}  



void *myThreadFun(void *arg)
{
	AVFormatContext *pFormatCtx2 = NULL;
    int             i, videoStream;  
	AVCodecContext  *pCodecCtx2 = NULL;   
	AVCodec         *pCodec2 = NULL; 
	AVFrame         *pFrame2 = NULL;   
	AVPacket        packet2;   
	int             frameFinished2;  
      
	AVDictionary    *optionsDict2 = NULL;   
    struct SwsContext *sws_ctx2 = NULL;

	SDL_Texture    *bmp2 = NULL;  
	SDL_Window     *screen2 = NULL;    
	SDL_Rect        rect2;  
    SDL_Event       event2; 	
	
	char *myplayer2video = (char*)arg;
	if(avformat_open_input(&pFormatCtx2, myplayer2video, NULL, NULL)!=0)  
	   return -1; // Couldn't open file  
    if(avformat_find_stream_info(pFormatCtx2, NULL)<0)  
        return -1; // Couldn't find stream information  	
	av_dump_format(pFormatCtx2, 0, myplayer2video, 0); 
	
    for(i=0; i<pFormatCtx2->nb_streams; i++)  
        if(pFormatCtx2->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {  
            videoStream=i;  
            break;  
        }  		
	pCodecCtx2=pFormatCtx2->streams[videoStream]->codec;  
	
	pCodec2=avcodec_find_decoder(pCodecCtx2->codec_id);  
    if(pCodec2==NULL) {  
        fprintf(stderr, "Unsupported codec!\n");  
        return -1; 
    }    	
		
    if(avcodec_open2(pCodecCtx2, pCodec2, &optionsDict2)<0)  
        return -1; // Could not open codec  
		
    pFrame2=av_frame_alloc(); 
      
    AVFrame* pFrameYUV2 = av_frame_alloc(); 
	 
    if( pFrameYUV2 == NULL )  
        return -1;  

    screen2 = SDL_CreateWindow("Player2",  
                              750,  
                              300,  
                              320,  240,    
                              SDL_WINDOW_OPENGL);                      
                              
   SDL_Renderer *renderer2 = SDL_CreateRenderer(screen2, -1, 0);          		
   bmp2 = SDL_CreateTexture(renderer2,SDL_PIXELFORMAT_YV12,SDL_TEXTUREACCESS_STREAMING,pCodecCtx2->width,pCodecCtx2->height);   // pCodecCtx2		
    
	sws_ctx2 =  
    sws_getContext  
    (  
     pCodecCtx2->width,  
     pCodecCtx2->height,  
     pCodecCtx2->pix_fmt,  
     pCodecCtx2->width,  
     pCodecCtx2->height,  
     AV_PIX_FMT_YUV420P,  
     SWS_BILINEAR,  
     NULL,  
     NULL,  
     NULL  
     );  

    int numBytes2 = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx2->width,  
                                  pCodecCtx2->height);  
    uint8_t* buffer2 = (uint8_t *)av_malloc(numBytes2*sizeof(uint8_t));  
      
    avpicture_fill((AVPicture *)pFrameYUV2, buffer2, AV_PIX_FMT_YUV420P,  
                   pCodecCtx2->width, pCodecCtx2->height);  
				   
				   
    rect2.x = 0;  
    rect2.y = 0;  
    rect2.w = pCodecCtx2->width;  
    rect2.h = pCodecCtx2->height;       

    while(av_read_frame(pFormatCtx2, &packet2)>=0) {   
		//Player2
		if(packet2.stream_index==videoStream) {  
            // Decode video frame  

			avcodec_decode_video2(pCodecCtx2, pFrame2, &frameFinished2,  
                                  &packet2);   					      

            if(frameFinished2) {  
  
                sws_scale  
                (  
                 sws_ctx2,  
                 (uint8_t const * const *)pFrame2->data,  
                 pFrame2->linesize,  
                 0,  
                 pCodecCtx2->height,  
                 pFrameYUV2->data,  
                 pFrameYUV2->linesize  
                 );  
				SDL_UpdateTexture( bmp2, &rect2, pFrameYUV2->data[0], pFrameYUV2->linesize[0] );    
                
                SDL_RenderClear( renderer2 );  
                SDL_RenderCopy( renderer2, bmp2, &rect2, &rect2 );
                SDL_RenderPresent( renderer2 );  
            }
            SDL_Delay(50);  

        }	 
        // Free the packet that was allocated by av_read_frame  		
		av_free_packet(&packet2);  // key point 
        SDL_PollEvent(&event2);  
        switch(event2.type) {  
            case SDL_QUIT:  
                SDL_Quit();  
                exit(0);  
                break;  
            default:  
                break;  
        } 
    } 
    
	SDL_DestroyTexture(bmp2); 
    av_free(pFrame2);  
    av_free(pFrameYUV2); 
	avcodec_close(pCodecCtx2); 
	avformat_close_input(&pFormatCtx2);  		
	 	
//	while(1)  for Testing
//	{
//		printf("Printing GeeksQuiz from Thread \n");
//		sleep(1);	
//	}

    return NULL;
}
  
int main(int argc, char *argv[]) {  

	pthread_t tid;
	char player2video;
	
    AVFormatContext *pFormatCtx = NULL;
	AVFormatContext *pFormatCtx2 = NULL;
    int             i, videoStream;  
    AVCodecContext  *pCodecCtx = NULL;
	AVCodecContext  *pCodecCtx2 = NULL;   
    AVCodec         *pCodec = NULL; 
	AVCodec         *pCodec2 = NULL; 
    AVFrame         *pFrame = NULL;
	AVFrame         *pFrame2 = NULL;   
    AVPacket        packet;
	AVPacket        packet2;   
    int             frameFinished;
	int             frameFinished2;  
    //float           aspect_ratio;  
      
    AVDictionary    *optionsDict = NULL;
	AVDictionary    *optionsDict2 = NULL;  
    struct SwsContext *sws_ctx = NULL;  
    struct SwsContext *sws_ctx2 = NULL;
    //SDL_CreateTexture();  
    SDL_Texture    *bmp = NULL;
	SDL_Texture    *bmp2 = NULL;  
    SDL_Window     *screen = NULL;
	SDL_Window     *screen2 = NULL;    
    SDL_Rect        rect;
	SDL_Rect        rect2;  
    SDL_Event       event; 
      
    if(argc < 2) {  
        fprintf(stderr, "Successfully generate the exe file!\n");  
        exit(1);  
    }  
    // Register all formats and codecs  
    av_register_all();  
      
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {  
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());  
        exit(1);  
    }  
      
    // Open video file  
    if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0)  
        return -1; // Couldn't open file          
   
    player2video = argv[2];  

    // Retrieve stream information  
    if(avformat_find_stream_info(pFormatCtx, NULL)<0)  
        return -1; // Couldn't find stream information  
    
    // Dump information about file onto standard error  
    av_dump_format(pFormatCtx, 0, argv[1], 0);

	
    // Find the first video stream  
    videoStream=-1;  
    for(i=0; i<pFormatCtx->nb_streams; i++)  
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {  
            videoStream=i;  
            break;  
        }  	
    if(videoStream==-1)  
        return -1; // Didn't find a video stream 

    // Get a pointer to the codec context for the video stream  
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;
   
    // Find the decoder for the video stream  
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL) {  
        fprintf(stderr, "Unsupported codec!\n");  
        return -1; // Codec not found  
    }    

    // Open codec  
    if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)  
        return -1; // Could not open codec  
      
    pFrame=av_frame_alloc();
      
    AVFrame* pFrameYUV = av_frame_alloc();
	 
    if( pFrameYUV == NULL )  
        return -1;     
        
      
    screen = SDL_CreateWindow("Player1",  
                              SDL_WINDOWPOS_UNDEFINED,  //SDL_WINDOWPOS_UNDEFINED   Horizon
                              SDL_WINDOWPOS_UNDEFINED,  //                          Vertical 500
                              320,  240,    			//pCodecCtx->width,  pCodecCtx->height,
                              SDL_WINDOW_OPENGL);  		// SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL
                                                                       
    SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, 0);  

      
    if(!screen) {  
        fprintf(stderr, "SDL: could not set video mode - exiting\n");  
        exit(1);  
    }  
     
    bmp = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_YV12,SDL_TEXTUREACCESS_STREAMING,pCodecCtx->width,pCodecCtx->height);

    
    sws_ctx =  
    sws_getContext  
    (  
     pCodecCtx->width,  
     pCodecCtx->height,  
     pCodecCtx->pix_fmt,  
     pCodecCtx->width,  
     pCodecCtx->height,  
     AV_PIX_FMT_YUV420P,  
     SWS_BILINEAR,  
     NULL,  
     NULL,  
     NULL  
     );  
         
    int numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width,  
                                  pCodecCtx->height);  
    uint8_t* buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));  
      
    avpicture_fill((AVPicture *)pFrameYUV, buffer, AV_PIX_FMT_YUV420P,  
                   pCodecCtx->width, pCodecCtx->height);       
    // Read frames and save first five frames to disk  

    i=0;  
      
    rect.x = 0;  
    rect.y = 0;  
    rect.w = pCodecCtx->width;  
    rect.h = pCodecCtx->height;  

    printf("Before Thread\n");
    pthread_create(&tid, NULL, myThreadFun, (void*)player2video);
    //pthread_join(tid, NULL);
    printf("After Thread\n");
    
      
    while(av_read_frame(pFormatCtx, &packet)>=0) {  
        // Is this a packet from the video stream?  
        if(packet.stream_index==videoStream) {  
            // Decode video frame  
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,  
                                  &packet);					      
            // Did we get a video frame?  
            if(frameFinished) {  
  
                sws_scale  
                (  
                 sws_ctx,  
                 (uint8_t const * const *)pFrame->data,  
                 pFrame->linesize,  
                 0,  
                 pCodecCtx->height,  
                 pFrameYUV->data,  
                 pFrameYUV->linesize  
                 );  
                SDL_UpdateTexture( bmp, &rect, pFrameYUV->data[0], pFrameYUV->linesize[0] );  
                             
                SDL_RenderClear( renderer );  
                SDL_RenderCopy( renderer, bmp, &rect, &rect );
                SDL_RenderPresent( renderer );  
            }  
            
            SDL_Delay(50);  

        }  
	
    
    }  

    SDL_DestroyTexture(bmp);     
    // Free the YUV frame  
    av_free(pFrame);  
    av_free(pFrameYUV); 
  
    // Close the codec  
    avcodec_close(pCodecCtx);  
      
    // Close the video file  
    avformat_close_input(&pFormatCtx);
        
    return 0;  
}  
