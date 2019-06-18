
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Bugra Sipahioglu -- Orcun Ozdemir");

static void searchChildrenRecursively(struct task_struct *task);
int p;
int childrenProcessIDs[100];
int childrenProcessIDCounter = 0;

static int startExecution(void) 
{
	printk(KERN_INFO "\n--->EXECUTION STARTED...\n");
	printk(KERN_INFO "\n--->PID PARAMETER USER ENTERED WAS: %d\n", p);

	

	searchChildrenRecursively(&init_task);
	printk(KERN_INFO "\n--->START PROGRAM RETURNING 0...\n");
	return 0;
}




void searchChildrenRecursively(struct task_struct *task)
{
	struct task_struct *child_task;
	struct task_strcut *bro_task;
	struct list_head *children_list;
	int isTargetedProcessFound = 0;
	int localMin = 1000000;
	struct task_struct* childrenTasksList[20];
	int ct = 0;

	printk(KERN_INFO "\n\n************Parent[%s , %d]\n",task->comm, task->pid);

	list_for_each(children_list, &task->children)
	{
		child_task = list_entry(children_list, struct task_struct, sibling);
		if(localMin < child_task->pid) 
		{
			localMin = child_task->pid;
		}
	}
    
    printk(KERN_INFO "\n\nOldest Child PID of the Parent[%d] is %d", task->pid, localMin);

	list_for_each(children_list, &task->children)
	{														   
		child_task = list_entry(children_list, struct task_struct, sibling);
		searchChildrenRecursively(child_task);

	}

}


static void endExecution(void)
{
	printk(KERN_INFO "\n--->EXECUTION ENDING... GOODBYE\n");
}

module_init(startExecution);
module_exit(endExecution);
module_param(p, int, 0);


