cmd_/home/zhen/Downloads/linux-5.9.3/drivers/drivers_study/tasklet/Module.symvers := sed 's/ko$$/o/' /home/zhen/Downloads/linux-5.9.3/drivers/drivers_study/tasklet/modules.order | scripts/mod/modpost -m -a   -o /home/zhen/Downloads/linux-5.9.3/drivers/drivers_study/tasklet/Module.symvers -e -i Module.symvers   -T -