#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


static char *ngx_http_mytest(ngx_conf_t *cf,
                             ngx_command_t *cmd,
                             void *conf);
static ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r);

static ngx_command_t ngx_http_mytest_commands[] = {
    {
        ngx_string("mytest"),
        NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF
            | NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,
        ngx_http_mytest,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL,
    },
    ngx_null_command,
};

static ngx_http_module_t ngx_http_mytest_module_ctx = {
    NULL,
    NULL,

    NULL,
    NULL,

    NULL,
    NULL,

    NULL,
    NULL,
};

ngx_module_t ngx_http_mytest_module = {
    NGX_MODULE_V1,
    &ngx_http_mytest_module_ctx,
    ngx_http_mytest_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING,
};

char *ngx_http_mytest(ngx_conf_t *cf,
                      ngx_command_t *cmd,
                      void *conf)
{
    ngx_http_core_loc_conf_t *clcf = NULL;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_mytest_handler;

    return NGX_CONF_OK;
}

ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r)
{
    return NGX_HTTP_NOT_ALLOWED;
}
