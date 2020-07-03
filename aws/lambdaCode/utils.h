#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include <curl/curl.h>

#define HTTP_PROTOCOL "http://"
#define RUNTIME_INVOCATION_PATH "/2018-06-01/runtime/invocation/"
#define NEXT_INVOCATION_PATH "next"
#define INVOCATION_RESPONSE_PATH "/response"
#define INVOCATION_ERROR_PATH "/error"
#define REQUEST_ID_REGEX "Lambda-Runtime-Aws-Request-Id:\\s{0,}([^)]*)\\\n"

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string*);
size_t writefunc(void *, size_t, size_t, struct string *);
void substring(char [], char [], int, int);

char curl_request(char*, int, char*, char*, char*);
char createURL(char*, int, char*, char*);

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }

  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s){
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

void substring(char s[], char sub[], int p, int l) {
   int c = 0;
   p++;l--;
   while (c < l) {
      sub[c] = s[p+c-1];
      c++;
   }
   sub[c] = '\0';
}

char createCurlRequest(
  char* curlURL,
  int curlMethodInt,
  char* curlPostFieldsString,
  char* curlResponseBuffer,
  char* curlResponseHeadersBuffer
){
  CURL *curl = curl_easy_init();
  if(curl) {
    struct string s;
    struct string headers;

    int requestIdPullCheck;
    int countryPullCheck;
    int regionPullCheck;

    regex_t requestIdPullRegex;
    regex_t countryPullRegex;
    regex_t regionPullRegex;
    size_t nmatch = 2;
    regmatch_t pmatch[2];

    CURLcode res;
    init_string(&s);
    init_string(&headers);

    //printf("we gonna call \n%s\n",curlURL);

    curl_easy_setopt(curl, CURLOPT_URL, curlURL);
    if(curlMethodInt == 1){
      // GET as you can see
      curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    } else {
      // POST
      curl_easy_setopt(curl, CURLOPT_POST, 1L);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen(curlPostFieldsString));
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, curlPostFieldsString);
    }

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headers);

    res = curl_easy_perform(curl);

    if(curlMethodInt == 1){

      char countryValueString[512];
      char regionValueString[512];

      requestIdPullCheck = regcomp(
        &requestIdPullRegex, 
        REQUEST_ID_REGEX, 
        REG_EXTENDED|REG_ICASE|REG_NEWLINE
      );

      countryPullCheck = regcomp(
        &countryPullRegex, 
        AKAMAI_EDGESCAPE_COUNTRY_PULL_REGEX, 
        REG_EXTENDED|REG_ICASE
      );

      regionPullCheck = regcomp(
        &regionPullRegex, 
        AKAMAI_EDGESCAPE_REGION_PULL_REGEX, 
        REG_EXTENDED|REG_ICASE
      );

      requestIdPullCheck = regexec(
        &requestIdPullRegex, 
        headers.ptr, 
        nmatch, 
        pmatch, 
        0
      );

      substring(
        headers.ptr,
        curlResponseHeadersBuffer,
        pmatch[1].rm_so,
        pmatch[1].rm_eo - pmatch[1].rm_so
      );

      countryPullCheck = regexec(
        &countryPullRegex, 
        s.ptr, 
        nmatch, 
        pmatch, 
        0
      );

      substring(
        s.ptr,
        countryValueString,
        pmatch[1].rm_so,
        pmatch[1].rm_eo - pmatch[1].rm_so
      );

      regionPullCheck = regexec(
        &regionPullRegex, 
        s.ptr, 
        nmatch, 
        pmatch, 
        0
      );

      substring(
        s.ptr,
        regionValueString,
        pmatch[1].rm_so,
        pmatch[1].rm_eo - pmatch[1].rm_so
      );

      strcat(curlResponseBuffer, countryValueString);
      strcat(curlResponseBuffer, regionValueString);

      //strcpy(curlResponseBuffer, s.ptr);

      regfree(&requestIdPullRegex);
      regfree(&countryPullRegex);
      regfree(&regionPullRegex);
    } 

    free(s.ptr);
    free(headers.ptr);
    curl_easy_cleanup(curl);
    return 0;
  } else {
    printf("helo ");
  }
}


char createURL(
  char* createURLBuffer,
  int urlCaseInt,
  char* lambdaRuntime,
  char* requestId
){
  stpcpy(
    stpcpy(
      stpcpy(
        createURLBuffer,
        HTTP_PROTOCOL
      ),
      lambdaRuntime
    ),
    RUNTIME_INVOCATION_PATH
  );

  switch (urlCaseInt) {
    case 1:
      strcat(createURLBuffer, NEXT_INVOCATION_PATH);
      break;
    case 2: case 3:
      strcat(createURLBuffer, requestId);
      if(urlCaseInt == 2){
        strcat(createURLBuffer, INVOCATION_RESPONSE_PATH);      
      } else {
        strcat(createURLBuffer, INVOCATION_ERROR_PATH);
      }
  }
}

