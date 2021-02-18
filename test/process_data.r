library(tidyverse)  
library(stringr)    # str_extract
library(reshape2)   # melt

################################################################################
#
# Load the data
#
cc_k <- read.table("u10-90-n10-k.txt")
cc_f <- read.table("u10-90-n10-freertos.txt")
#cc <- read.table("u10-90-n10-i30-c10-ss-2.txt")
cc1 <- read.table("u10-50_n10_i30_t1-100_ss-cycles.txt")
cc2 <- read.table("u60-90_n10_i30_t1-100_ss-cycles.txt")
cc1_k <- read.table("u10-50_n10_i30_t1-100_k-cycles.txt")
cc2_k <- read.table("u60-90_n10_i30_t1-100_k-cycles.txt")
cc <- rbind(cc1, cc2, cc1_k, cc2_k)
cc$u <- strtoi(gsub("^u*", "", str_extract(cc$V33, "u[0-9]*")))
cc$n <- strtoi(gsub("^n*", "", str_extract(cc$V33, "n[0-9]*")))
cc$V33 <- NULL
colnames(cc) <- c("id", "task", 1:30, "slack","method","calc","u","n")
rm <- melt(cc, id.vars=c("n","u","id","task","slack","method","calc")) # melt for easier plotting


################################################################################
#
# Summary statistics
#

# using dplyr library
# reminder to myself: %>% is the pipe operator
library(dplyr)
rm[ which(rm$slack=='ss' & rm$calc=='d'), ][c('u','value')] %>%
  group_by(u) %>%
  summarise(mean_value=mean(value), sd=sd(value), min_value=min(value), max_value=max(value))

################################################################################
#
# Plots
#

rm$m <- paste(cc$slack, cc$calc, sep='-')

rm[ which(rm$m=='ss-d'), ][c('u', 'task', 'variable', 'value')] %>%
  group_by(u, task, variable) %>%
  summarise(mean_value=mean(value), sd=sd(value), min_value=min(value), max_value=max(value)) %>%
  ggplot(aes(x=variable, y=mean_value, group=task)) +
  geom_line() +
  geom_point(size=2.5) +
  theme_bw() +
  facet_wrap(~u, nrow=3)

rm[ which(rm$m=='ss-d' & rm$u==70), ][c('task', 'variable', 'value')] %>%
  group_by(variable, task) %>%
  summarise(mean_value=mean(value), sd=sd(value), min_value=min(value), max_value=max(value)) %>%
  ggplot(aes(x=variable, y=mean_value, group=task)) +
  geom_line() +
  geom_point(size=2.5) +
  theme_bw()

rm[c('m', 'u', 'task', 'variable', 'value')] %>%
  group_by(u, m) %>%
  ggplot(aes(x=variable, y=value, group=m, shape=m)) +
  geom_point() +
  theme_bw() +
  scale_x_continuous(breaks = seq(0, 9, by=1)) +
  labs(
    x = "U.F.",
    y = "CPU clyes",
    shape = "Test",
    title = paste(
      "Context switch mean cost per U.F."
    )
  ) +
  facet_wrap(~u, nrow=3) +
  theme(plot.title = element_text(hjust = 0.5))

# Context switch cost per U.F
rm[c('m', 'u','value')] %>%
  group_by(u, m) %>%
  summarise(mean_value=mean(value), sd=sd(value), min_value=min(value), max_value=max(value)) %>%
  ggplot(aes(x=u, y=mean_value, group=m, shape=m)) +
  geom_line() +
  geom_point(size=2.5) +
  theme_bw() +
  scale_x_continuous(breaks = seq(10, 90, by=10)) +
  labs(
    x = "U.F.",
    y = "CPU clyes",
    shape = "Test",
    title = paste(
      "Context switch mean cost per U.F."
    )
  ) +
  theme(plot.title = element_text(hjust = 0.5))

# context switch cost per task
rm[c('m', 'u', 'task','value')] %>%
  group_by(u, task, m) %>%
  summarise(mean_value=mean(value), sd=sd(value), min_value=min(value), max_value=max(value)) %>%
  ggplot() +
  geom_line(mapping = aes(x=task, y=mean_value, linetype=factor(u), group=u)) +
  geom_point(mapping = aes(x=task, y=mean_value)) +
  theme_bw() +
  scale_x_continuous(breaks = seq(0, 9, by=1)) +
  labs(
    title="Context switch mean cost per task",
    x = "Task",
    y = "CPU cycles",
    linetype = "U.F."
  ) +
  theme(plot.title = element_text(hjust = 0.5))
    
# task's context switch cost per U.F
rm[c('m', 'u', 'task','value')] %>%
  group_by(u, task, m) %>%
  summarise(mean_value=mean(value), sd=sd(value), min_value=min(value), max_value=max(value)) %>%
  ggplot() +
  geom_line(mapping = aes(x=u, y=mean_value, linetype=factor(task), group=task)) +
  geom_point(mapping = aes(x=u, y=mean_value)) +
  theme_bw() +
  scale_x_continuous(breaks = seq(10, 90, by=10)) +
  labs(
    title="Context switch mean cost per task",
    x = "U.F.",
    y = "CPU cycles",
    linetype = "Task"
  ) +
  theme(plot.title = element_text(hjust = 0.5))

rm[c('m', 'u', 'task','value')] %>%
  group_by(u, task, m) %>%
  summarise(mean_value=mean(value), sd=sd(value), min_value=min(value), max_value=max(value)) %>%
  ggplot() +
  geom_line(mapping = aes(x=task, y=mean_value, group=u)) +
  geom_point(mapping = aes(x=task, y=mean_value, group=u)) +
  theme_bw() +
  facet_wrap(~u, nrow=3)

rm[c('m', 'u','value')] %>% group_by(u,m) %>% summarise(mean_value=mean(value))

# mean of each test grouped by utilization factor
aggdata <- aggregate(rm, by=list(rm$u,rm$slack,rm$calc), FUN=mean, na.rm=TRUE)

################################################################################
#
# Plots
#

# scatterplot
filter_f <- rm[ which(rm$slack=='noss'), ][c('u','value')]
filter_k <- rm[ which(rm$slack=='ss' & rm$calc=='k'), ][c('u','value')]
filter_s <- rm[ which(rm$slack=='ss' & rm$calc=='d'), ][c('u','value')]
plot(rm$u, rm$value, main="Context switch costs by U.F", xlab="Task", ylab="CPU cycles", xaxt="n")
axis(1, at=c(10,20,30,40,50,60,70,80,90), labels=c("10%","20%","30%","40%","50%","60%","70%","80%","90%"))
abline(lm(rm$value~rm$task), col="red")
lines(lowess(rm$task,rm$value), col="blue") # lowess line (x,y) 

# scatterplot and mean value of context switch costs per U.F when using slack
filter_s <- rm[ which(rm$slack=='ss' & rm$calc=='d'), ][c('u','value')]
aggdata_s <- aggregate(filter_s, by=list(filter_s$u), FUN=mean, na.rm=TRUE)
plot(filter_s$u, filter_s$value, xlab="U.F", ylab="CPU cycles", xaxt="n")
lines(aggdata_s$Group.1, aggdata_s$value, type="l", lty=1)
axis(1, at=c(10,20,30,40,50,60,70,80,90), labels=c("10%","20%","30%","40%","50%","60%","70%","80%","90%"))
title("Context switch costs by U.F. when using SS")

# scatterplot and mean value of context switch costs per task when using slack
filter_s <- rm[ which(rm$slack=='ss' & rm$calc=='d'), ][c('task','value')]
aggdata_s <- aggregate(filter_s, by=list(filter_s$task), FUN=mean, na.rm=TRUE)
plot(filter_s$task, filter_s$value, xlab="Task", ylab="CPU cycles", xaxt="n")
lines(aggdata_s$Group.1, aggdata_s$value, type="l", lty=1)
axis(1, at=c(0,1,2,3,4,5,6,7,8,9), labels=c("1","2","3","4","5","6","7","8","9", "10"))
title("Context switch costs by task when using SS")

# scatterplot and mean value of context switch costs per U.F when using K
filter_k <- rm[ which(rm$slack=='ss' & rm$calc=='k'), ][c('u','value')]
aggdata_k <- aggregate(filter_k, by=list(filter_k$u), FUN=mean, na.rm=TRUE)
plot(filter_k$u, filter_k$value, xlab="U.F.", ylab="CPU cycles", xaxt="n")
lines(aggdata_k$Group.1, aggdata_k$value, type="l", lty=1)
axis(1, at=c(10,20,30,40,50,60,70,80,90), labels=c("10%","20%","30%","40%","50%","60%","70%","80%","90%"))
title("Context switch costs by U.F. when using K")

# scatterplot and mean value of context switch costs per task when using K
filter_k <- rm[ which(rm$slack=='ss' & rm$calc=='k'), ][c('task','value')]
aggdata_k <- aggregate(filter_k, by=list(filter_k$task), FUN=mean, na.rm=TRUE)
plot(filter_k$task, filter_k$value, xlab="Task", ylab="CPU cycles", xaxt="n")
lines(aggdata_k$Group.1, aggdata_k$value, type="l", lty=1)
axis(1, at=c(0,1,2,3,4,5,6,7,8,9), labels=c("1","2","3","4","5","6","7","8","9", "10"))
title("Context switch costs by task when using K")

# scatterplot and mean value of context switch costs per U.F of FreeRTOS
filter_f <- rm[ which(rm$slack=='noss',), ][c('u','value')]
aggdata_f <- aggregate(filter_f, by=list(filter_f$u), FUN=mean, na.rm=TRUE)
plot(filter_f$u, filter_f$value, xlab="U.F.", ylab="CPU cycles", xaxt="n")
lines(aggdata_f$Group.1, aggdata_f$value, type="l", lty=1)
axis(1, at=c(10,20,30,40,50,60,70,80,90), labels=c("10%","20%","30%","40%","50%","60%","70%","80%","90%"))
title("Context switch costs by U.F. of FreeRTOS")

# scatterplot and mean value of context switch costs per task of FreeRTOS
filter_f <- rm[ which(rm$slack=='noss',), ][c('task','value')]
aggdata_f <- aggregate(filter_f, by=list(filter_f$task), FUN=mean, na.rm=TRUE)
plot(filter_f$task, filter_f$value, xlab="Task", ylab="CPU cycles", xaxt="n")
lines(aggdata_f$Group.1, aggdata_f$value, type="l", lty=1)
axis(1, at=c(0,1,2,3,4,5,6,7,8,9), labels=c("1","2","3","4","5","6","7","8","9", "10"))
title("Context switch costs by task of FreeRTOS")

# line plot of context switch mean values when using slack, k and unmodified freertos
filter_f <- rm[ which(rm$slack=='noss'), ][c('u','value')]
filter_k <- rm[ which(rm$slack=='ss' & rm$calc=='k'), ][c('u','value')]
filter_s <- rm[ which(rm$slack=='ss' & rm$calc=='d'), ][c('u','value')]
aggdata_f <- aggregate(filter_f, by=list(filter_f$u), FUN=mean, na.rm=TRUE)
aggdata_k <- aggregate(filter_k, by=list(filter_k$u), FUN=mean, na.rm=TRUE)
aggdata_s <- aggregate(filter_s, by=list(filter_s$u), FUN=mean, na.rm=TRUE)
plot(aggdata_s$Group.1, aggdata_s$value, xlab="U.F.", ylab="CPU cycles", type="o", ylim=c(0,3000),pch=15, lty=1, xaxt="n")
lines(aggdata_k$Group.1, aggdata_k$value, type="o", pch=16, lty=1)
lines(aggdata_f$Group.1, aggdata_f$value, type="o", pch=17, lty=1)
axis(1, at=c(10,20,30,40,50,60,70,80,90), labels=c("10%","20%","30%","40%","50%","60%","70%","80%","90%"))
legend("topleft", inset=.05, legend=c("SS", "K", "Vanilla"), pch=c(15,16,17))
title("Mean context switch cost by U.F.")

# ussing aggregate function and simple plot
filter_f <- rm[ which(rm$slack=='noss'), ][c('u','value')]
filter_k <- rm[ which(rm$slack=='ss' & rm$calc=='k'), ][c('u','value')]
filter_s <- rm[ which(rm$slack=='ss' & rm$calc=='d'), ][c('u','value')]
aggdata_f <- aggregate(filter_f, by=list(filter_f$u), FUN=max, na.rm=TRUE)
aggdata_k <- aggregate(filter_k, by=list(filter_k$u), FUN=max, na.rm=TRUE)
aggdata_s <- aggregate(filter_s, by=list(filter_s$u), FUN=max, na.rm=TRUE)
plot(aggdata_s$Group.1, aggdata_s$value, xlab="U.F.", ylab="CPU cycles", type="o", ylim=c(0,15000),pch=15, lty=1, xaxt="n")
lines(aggdata_k$Group.1, aggdata_k$value, xlab="U.F.", ylab="CPU cycles", type="o", pch=16, lty=1)
lines(aggdata_f$Group.1, aggdata_f$value, xlab="U.F", ylab="CPU cycles", type="o", pch=17, lty=1)
axis(1, at=c(10,20,30,40,50,60,70,80,90), labels=c("10%","20%","30%","40%","50%","60%","70%","80%","90%"))
legend("topleft", inset=.05, legend=c("SS", "K", "Vanilla"), pch=c(15,16,17))
title("Worst context switch cost by U.F.")

filter_f <- rm[ which(rm$slack=='noss'), ]
filter_k <- rm[ which(rm$slack=='ss' & rm$calc=='k'), ][c('task','value')]
filter_s <- rm[ which(rm$slack=='ss' & rm$calc=='d'), ][c('task','value')]
aggdata_f <- aggregate(filter_f, by=list(filter_f$task), FUN=mean, na.rm=TRUE)
aggdata_k <- aggregate(filter_k, by=list(filter_k$task), FUN=mean, na.rm=TRUE)
aggdata_s <- aggregate(filter_s, by=list(filter_s$task), FUN=mean, na.rm=TRUE)
plot(aggdata_s$Group.1, aggdata_s$value, xlab="Task", ylab="CPU cycles", type="o", ylim=c(0,4000),pch=15, lty=1, xaxt="n")
lines(aggdata_k$Group.1, aggdata_k$value, type="o", pch=16, lty=1)
lines(aggdata_f$Group.1, aggdata_f$value, type="o", pch=17, lty=1)
axis(1, at=c(0,1,2,3,4,5,6,7,8,9), labels=c("1","2","3","4","5","6","7","8","9","10"))
legend("topleft", inset=.05, legend=c("SS", "K", "Vanilla"), pch=c(15,16,17))
title("Context switch cost by task")

# using psych library
library(psych)
filter_s <- rm[ which(rm$slack=='ss' & rm$calc=='d'), ][c('u','value')]
describeBy(filter_s, (filter_s$u), data=(filter_s$value))

summary(filter_s)

library(ggplot2)
library(dplyr)
rm[ which(rm$slack=='ss' & rm$calc=='d'), ][c('u','value')] %>%
  group_by(u) %>%
  summarise(mean_value=mean(value), sd=sd(value), min_value=min(value), max_value=max(value)) %>%
  ggplot(aes(x = u, y = mean_value, fill = mean_value)) +
    geom_line(stat = "identity") +
    theme_bw() +
    labs(
      x = "U.F.",
      y = "CPU clyes",
      title = paste(
        "Context switch mean cost per U.F."
      )
    )
