#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include "syswatch.h"

/* --- Simple INI-like config parser (key=value) --- */
static void trim(char *s){
    char *p=s; while(*p && (*p==' '||*p=='\t')) p++;
    if(p!=s) memmove(s,p,strlen(p)+1);
    size_t n=strlen(s);
    while(n>0 && (s[n-1]=='\n'||s[n-1]=='\r'||s[n-1]==' '||s[n-1]=='\t')) s[--n]=0;
}

int load_config(const char *path, sw_config_t *cfg){
    memset(cfg,0,sizeof(*cfg));
    cfg->sample_interval_sec = 5;
    strcpy(cfg->bind_addr, "0.0.0.0");
    cfg->bind_port = 9090;
    strcpy(cfg->metrics_log, "syswatch_metrics.log");

    FILE *f = fopen(path,"r"); if(!f){ perror("config"); return -1; }
    char line[512];
    while(fgets(line,sizeof(line),f)){
        trim(line); if(line[0]=='#'||line[0]==0) continue;
        char *eq = strchr(line,'=');
        if(!eq) continue; *eq=0;
        char *k=line,*v=eq+1; trim(k); trim(v);
        if(strcmp(k,"mount") == 0 && cfg->mount_count < MAX_MOUNTS)
            strncpy(cfg->mount_points[cfg->mount_count++], v, MAX_PATH-1);
        else if(strcmp(k,"log") == 0 && cfg->log_count < MAX_LOGS)
            strncpy(cfg->log_paths[cfg->log_count++], v, MAX_PATH-1);
        else if(strcmp(k,"metrics_log")==0) strncpy(cfg->metrics_log, v, MAX_PATH-1);
        else if(strcmp(k,"sample_interval")==0) cfg->sample_interval_sec = atoi(v);
        else if(strcmp(k,"bind_addr")==0) strncpy(cfg->bind_addr, v, sizeof(cfg->bind_addr)-1);
        else if(strcmp(k,"bind_port")==0) cfg->bind_port = atoi(v);
    }
    fclose(f);
    return 0;
}

/* --- Dedicated signal thread using sigwait --- */
static void *signal_thread(void *arg){
    sw_ctx_t *ctx = (sw_ctx_t*)arg;
    sigset_t set; sigemptyset(&set);
    sigaddset(&set, SIGTERM); sigaddset(&set, SIGINT);
    sigaddset(&set, SIGUSR1); sigaddset(&set, SIGHUP);

    int sig;
    while(atomic_load(ctx->running) && sigwait(&set, &sig)==0){
        if(sig==SIGTERM || sig==SIGINT){
            fprintf(stderr, "[signal] graceful shutdown\n");
            atomic_store(ctx->running, false);
        } else if(sig==SIGUSR1){
            system_metrics_t snap;
            pthread_mutex_lock(&ctx->metrics->data_lock);
            snap = *ctx->metrics;
            pthread_mutex_unlock(&ctx->metrics->data_lock);
            fprintf(stderr, "[signal] dump: ts=%llu cpu=%.2f mem=%.2f disk=%.2f\n",
                    snap.ts, snap.cpu_usage, snap.memory_usage, snap.disk_usage);
        } else if(sig==SIGHUP){
            fprintf(stderr, "[signal] (noop) reload requested\n");
            /* could re-read config here */
        }
    }
    return NULL;
}

static void usage(const char *p){
    fprintf(stderr,"Usage: %s -c <config>\n", p);
}

int main(int argc, char **argv){
    const char *cfg_path = NULL;
    int opt; while((opt=getopt(argc,argv,"c:h"))!=-1){
        if(opt=='c') cfg_path=optarg;
        else { usage(argv[0]); return 1; }
    }
    if(!cfg_path){ usage(argv[0]); return 1; }

    sw_config_t cfg; if(load_config(cfg_path, &cfg)!=0) return 2;

    system_metrics_t metrics = {0};
    pthread_mutex_init(&metrics.data_lock, NULL);
    atomic_bool running = true;

    sw_ctx_t ctx = { .metrics=&metrics, .cfg=&cfg, .running=&running };

    /* block signals in all threads, then create a dedicated signal waiter */
    sigset_t set; sigemptyset(&set);
    sigaddset(&set, SIGTERM); sigaddset(&set, SIGINT);
    sigaddset(&set, SIGUSR1); sigaddset(&set, SIGHUP);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    pthread_t t_metrics, t_logs, t_net, t_sig;
    pthread_create(&t_metrics, NULL, metrics_thread, &ctx);
    pthread_create(&t_logs,    NULL, logwatch_thread, &ctx);
    pthread_create(&t_net,     NULL, net_thread, &ctx);
    pthread_create(&t_sig,     NULL, signal_thread, &ctx);

    /* Wait until signal thread flips running=false */
    pthread_join(t_sig, NULL);

    /* join workers */
    pthread_join(t_metrics, NULL);
    pthread_join(t_logs, NULL);
    pthread_join(t_net, NULL);

    pthread_mutex_destroy(&metrics.data_lock);
    return 0;
}