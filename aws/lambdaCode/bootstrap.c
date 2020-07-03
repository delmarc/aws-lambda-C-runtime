/*
  brining in all lower env specific headers
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"

/*
  These are the certain environment variable
  names we want to be picking up for use in the
  lambda code...
*/
#define _HANDLER "_HANDLER"
#define AWS_LAMBDA_RUNTIME_API "AWS_LAMBDA_RUNTIME_API"

/*
  certain events that we want to shortcut
*/
#define nextInvocationRequestNumber 1
#define invocationResponseNumber 2
#define invocationErrorNumber 3

int main(void) {
  /*
    First we need to call for the request's payload
  */
  char *lambdaRuntimeAPI = getenv(AWS_LAMBDA_RUNTIME_API);
  char *handlerName = getenv(_HANDLER);
  char nextInvocationURLBuffer[128] = "";
  char invocationResponseURLBuffer[128] = "";

  char nextInvocationCurlResponseBuffer[2048] = "";
  char nextInvocationCurlResponseHeadersBuffer[2048] = "";

  if( lambdaRuntimeAPI == NULL ){
    printf("cant get the API");
    return 1;
  }

  while(1){

    createURL(
      nextInvocationURLBuffer,
      nextInvocationRequestNumber,
      lambdaRuntimeAPI,
      ""
    );

    createCurlRequest(
      nextInvocationURLBuffer,
      1,
      "",
      nextInvocationCurlResponseBuffer,
      nextInvocationCurlResponseHeadersBuffer
    );

    createURL(
      invocationResponseURLBuffer,
      invocationResponseNumber,
      lambdaRuntimeAPI,
      nextInvocationCurlResponseHeadersBuffer
    );

    createCurlRequest(
      invocationResponseURLBuffer,
      2,
      nextInvocationCurlResponseBuffer,
      nextInvocationCurlResponseBuffer,
      nextInvocationCurlResponseHeadersBuffer
    );

    nextInvocationURLBuffer = "";
    invocationResponseURLBuffer = "";

    nextInvocationCurlResponseBuffer = "";
    nextInvocationCurlResponseHeadersBuffer = "";

  }

}

