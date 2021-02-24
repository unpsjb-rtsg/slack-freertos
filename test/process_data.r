library(tidyverse)  # dyplr, ggplot2, etc.
library(stringr)    # str_extract
library(reshape2)   # melt

################################################################################
#
# Load and prepare the data
#
cc1 <- read.table("u10-30_n10_i30_t001-100_ss_cycles.txt")
cc2 <- read.table("u10-30_n10_i30_t101-200_ss_cycles.txt")
cc3 <- read.table("u10-30_n10_i30_t201-300_ss_cycles.txt")
cc4 <- read.table("u40-60_n10_i30_t001-100_ss_cycles.txt")
cc5 <- read.table("u40-60_n10_i30_t101-200_ss_cycles.txt")
cc6 <- read.table("u40-60_n10_i30_t201-300_ss_cycles.txt")
cc7 <- read.table("u70-90_n10_i30_t001-100_ss_cycles.txt")
cc8 <- read.table("u70-90_n10_i30_t101-200_ss_cycles.txt")
cc9 <- read.table("u70-90_n10_i30_t201-300_ss_cycles.txt")

cc1k <- read.table("u10-30_n10_i30_t001-100_k_cycles.txt")
cc2k <- read.table("u10-30_n10_i30_t101-200_k_cycles.txt")
cc3k <- read.table("u10-30_n10_i30_t201-300_k_cycles.txt")
cc4k <- read.table("u40-60_n10_i30_t001-100_k_cycles.txt")
cc5k <- read.table("u40-60_n10_i30_t101-200_k_cycles.txt")
cc6k <- read.table("u40-60_n10_i30_t201-300_k_cycles.txt")
cc7k <- read.table("u70-90_n10_i30_t001-100_k_cycles.txt")
cc8k <- read.table("u70-90_n10_i30_t101-200_k_cycles.txt")
cc9k <- read.table("u70-90_n10_i30_t201-300_k_cycles.txt")

cc <- rbind(cc1, cc2, cc3, cc4, cc5, cc6, cc7, cc8, cc9,
            cc1k, cc2k, cc3k, cc4k, cc5k, cc6k, cc7k, cc8k, cc9k)
cc$u <- strtoi(gsub("^u*", "", str_extract(cc$V33, "u[0-9]*")))
cc$n <- strtoi(gsub("^n*", "", str_extract(cc$V33, "n[0-9]*")))
cc$V33 <- NULL
colnames(cc) <- c("id", "task", 1:30, "slack","method","calc","u","n")
rm <- melt(cc, id.vars=c("n","u","id","task","slack","method","calc")) # melt for easier plotting
rm$m <- paste(cc$slack, cc$calc, sep='-')
rm$m <- factor(rm$m, levels=c("ss-d", "ss-k"), labels=c("Online", "Counters"))

################################################################################
#
# Summary statistics
#

# reminder to myself: %>% is the pipe operator
rm[ which(rm$m=='ss-d'), ][c('u','value')] %>%
  group_by(u) %>%
  summarise(mean_value=mean(value), sd=sd(value), min_value=min(value), max_value=max(value))

################################################################################
#
# Plots
#

# lineplots with the (mean, max) context switch cost by u.f for all tasks.
sgdata1 <- rm[ which(rm$task>=0 & rm$m=="Online"), ][c('u', 'm', 'variable', 'value')] %>%
  group_by(u, m) %>% summarise(value=max(value))
sgdata1$desc = "Online worst"
sgdata2 <- rm[ which(rm$task>=0 & rm$m=="Online"), ][c('u', 'm', 'variable', 'value')] %>%
  group_by(u, m) %>% summarise(value=mean(value))
sgdata2$desc = "Online mean"
sgdata3 <- rm[ which(rm$task>=0 & rm$m=="Counters"), ][c('u', 'm', 'variable', 'value')] %>%
  group_by(u, m) %>% summarise(value=max(value))
sgdata3$desc = "Counters worst"
sgdata <- rbind(sgdata1, sgdata2, sgdata3)
ggplot(sgdata, aes(x=u, y=value, group=desc)) +
  geom_line() +
  geom_point(aes(shape=desc), size=3) +
  scale_x_continuous(breaks = seq(10, 90, by=10)) +
  scale_y_continuous(trans='log10') +
  #geom_label(aes(x=u, y=value, label = sprintf("%1.0f", value)), nudge_y = 0.1, alpha = 0.5) +
  labs(
    x = "U.F.",
    y = "CPU cycles",
    title = paste("Context switch cost"),
    shape = "Calculation cost"
  ) +
  theme_bw() +
  theme(
    plot.title = element_text(hjust = 0.5),
  )

# lineplots with the context switch cost by release for al u.f, grouped by task
rm[ which(rm$m=='Online' & rm$task > 4 & rm$u>50), ][c('u', 'task', 'variable', 'value')] %>%
  group_by(u, task, variable) %>%
  summarise(mean_value=mean(value), sd=sd(value), min_value=min(value), max_value=max(value)) %>%
  ggplot(aes(x=variable, y=max_value, group=task)) +
  geom_line() +
  geom_point(aes(shape=factor(task)), size=1.5) +
  scale_x_discrete(breaks = c(1, 5, 10, 15, 20, 25, 30)) +
  #scale_y_continuous(trans='log10') +
  labs(
    x = "Instance",
    y = "CPU cycles",
    title = paste(
      "Maximum context switch costs of tasks 6 to 10"
    ),
    shape="Task"
  ) +
  theme_bw() +
  theme(
    plot.title = element_text(hjust = 0.5),
  ) +
  facet_wrap(~u, nrow=2)

# scatterplot of each release context-switch cost for a specific u.f. and task
rm[ which(rm$m=='ss-d' & rm$u==90 & rm$task==9), ][c('variable', 'value')] %>%
  ggplot(aes(x=variable, y=value)) +
  geom_point(size=2.5) +
  theme_bw() +
  labs(
    x = "Instance",
    y = "CPU clyes",
    title = paste(
      "Context switch cost per instance"
    )
  )

# linepoint of each release context-switch cost for a specific u.f. and task
rm[ which(rm$m=='ss-d' & rm$u==90 & rm$task==9), ][c('variable', 'value')] %>%
  group_by(variable) %>%
  summarise(mean_value=mean(value), sd=sd(value), min_value=min(value), max_value=max(value)) %>%
  ggplot(aes(x=variable, y=max_value, group=1)) +
  geom_line() +
  geom_point(size=2.5) +
  theme_bw() +
  labs(
    x = "Instance",
    y = "CPU cycles",
    title = paste(
      "Context switch cost per instance"
    )
  )

# linepoint of each release context-switch cost for a task for all u.f
rm[ which(rm$m=='ss-d' & rm$task==9), ][c('variable', 'u', 'value')] %>%
  group_by(variable, u) %>%
  summarise(mean_value=mean(value), sd=sd(value), min_value=min(value), max_value=max(value)) %>%
  ggplot(aes(x=variable, y=mean_value, group=u, linetype=factor(u))) +
  geom_line() +
  #geom_point(size=2.5) +
  theme_bw() +
  #geom_text(aes(label = u)) +
  labs(
    x = "Instance",
    y = "CPU clyes",
    title = paste(
      "Context switch cost per instance"
    )
  )

# Barplot which shows how much of the time-slice is used per U.F.
# Change the `which` for other tasks or the total (>0).
library(ggfittext)
rm[ which(rm$m=='Online' & rm$task > 4), ][c('u', 'value')] %>%
  mutate(pct=round(value/96000,2)) %>%
  mutate(category=cut(pct, breaks=c(-Inf, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, +Inf), 
                           labels=c("< 5%", "< 10%", "< 15%", "< 20%", "< 25%", "< 30%", ">"))) %>%
  group_by(u, category) %>%
  summarise(sales = n()) %>%
  mutate(c=sum(sales)) %>%
  mutate(pct2=round(sales/c,6)) %>%
  ggplot(aes(fill=category, y=pct2, x=u)) + 
  geom_col(colour = "black", position="fill") +
  geom_bar_text(aes(label=paste0(sprintf("%1.0f", pct2*100),"%")), position = "stack", reflow = TRUE) +
  theme_bw() +
  scale_fill_grey(start=0.9, end=0.1) +
  #scale_fill_brewer(palette = "Pastel2") +
  scale_x_continuous(breaks = seq(10, 90, by=10), labels = c("10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%")) +
  scale_y_continuous(labels = scales::percent) +
  labs(
    x = "U.F.",
    y = "% of cases",
    title = paste("Slack calculation cost as % of the time-slice for tasks 6 to 10"),
    fill = "% of time-slice"
  ) +
  theme(
    plot.title = element_text(hjust = 0.5),
  )

# Barplot which shows how much of the time-slice is used per task.
# Change the `which` for other tasks or the total (>0).
library(ggfittext)
rm[ which(rm$m=='Online'), ][c('task', 'value')] %>%
  mutate(pct=round(value/96000,2)) %>%
  mutate(category=cut(pct, breaks=c(-Inf, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, +Inf), 
                      labels=c("< 5%", "< 10%", "< 15%", "< 20%", "< 25%", "< 30%", ">"))) %>%
  group_by(task, category) %>%
  summarise(sales = n()) %>%
  mutate(c=sum(sales)) %>%
  mutate(pct2=round(sales/c,6)) %>%
  ggplot(aes(fill=category, y=pct2, x=task)) + 
  geom_col(colour = "black", position="fill") +
  geom_bar_text(aes(label=paste0(sprintf("%1.0f", pct2*100),"%")), position = "stack", reflow = TRUE)+#aes(label=paste0(sprintf("%1.0f", pct2*100),"%"))) +
  theme_bw() +
  scale_fill_grey(start=0.9, end=0.1) +
  #scale_fill_brewer(palette = "Pastel2") +
  scale_x_continuous(breaks = seq(0, 9, by=1), labels = c("1", "2", "3", "4", "5", "6", "7", "8", "9", "10")) +
  scale_y_continuous(labels = scales::percent) +
  labs(
    x = "Task",
    y = "% of cases",
    title = paste("Slack calculation cost as % of the time-slice"),
    fill = "% of time-slice"
  ) +
  theme(
    plot.title = element_text(hjust = 0.5),
  )

rm[ which(rm$m=='Online' & rm$u==90 & rm$task==9), ][c('value')] %>%
  ggplot() +
  geom_histogram(mapping = aes(x=value), colour="black", fill="white", binwidth=1000) +
  theme_bw() +
  labs(
    x = "CPU cycles",
    y = "Count",
    title = paste(
      "Context switch cost per instance"
    )
  )

rm[ which(rm$m=='Online' & rm$u==90), ][c('task', 'variable', 'value')] %>%
  group_by(variable, task) %>%
  summarise(mean_value=mean(value), sd=sd(value), min_value=min(value), max_value=max(value)) %>%
  ggplot(aes(x=variable, y=mean_value, group=task)) +
  geom_line() +
  geom_point(size=2.5) +
  theme_bw() +
  labs(
    x = "Instance",
    y = "CPU clyes",
    title = paste(
      "Context switch cost per instance"
    )
  )

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

# context switch cost per task
rm[which(rm$m=='Counters'),][c('m', 'u', 'task','value')] %>%
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
rm[which(rm$m=='Online'),][c('m', 'u', 'task','value')] %>%
  group_by(u, task, m) %>%
  summarise(mean_value=mean(value), sd=sd(value), min_value=min(value), max_value=max(value)) %>%
  ggplot() +
  geom_line(mapping = aes(x=u, y=max_value, linetype=factor(task), group=task)) +
  geom_point(mapping = aes(x=u, y=max_value)) +
  theme_bw() +
  scale_x_continuous(breaks = seq(10, 90, by=10)) +
  labs(
    title="Context switch mean cost per task",
    x = "U.F.",
    y = "CPU cycles",
    linetype = "Task"
  ) +
  theme(plot.title = element_text(hjust = 0.5))

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
