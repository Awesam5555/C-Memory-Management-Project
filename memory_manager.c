#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/pid.h>
#include <linux/swap.h>
#include <linux/swapops.h>
#include <linux/uaccess.h>
#include <linux/pgtable.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Samson Mathias");
MODULE_DESCRIPTION("CSE330 Project 5");

// Parameters for the kernel module and allows us to load the pid into the kernel for testing and for the code to actually work 
static int pid = -1;
module_param(pid, int, 0);
MODULE_PARM_DESC(pid, "Process ID");

// so, I used Dulcet's link in the discord on why I should load in adresses and this part of the code allows me to add into  adresses and basically makes sure i don't have to manually add in adresses which is imporatn in the github 
static unsigned long addr = 0;
module_param(addr, ulong, 0);
MODULE_PARM_DESC(addr, "Virtual address");

static void fix_address(void) {
	// struct declarations 
    struct pid *pid_struct;
    struct task_struct *task;
    struct mm_struct *mm;
    // the TA said this shouldn't be a problem but this is the page directory table and its initliaization which I got straight from the project 5 documentation, in case it looks similiar. 
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;

    // as a more general comment the below code with all the KERN_ERR, is just something I used more as error checking/error debugging. The TA said it was fine for me to have it and all it does is just check for errors in any part of the code i wrote and I just used it as helpful ness when i was running it and couldn't find where certain things were going wrong in my code. And don't worry my other print statements, the ones that actually matter in the project description are in the right fromat. 
	// finds the PID and then puts that into the pid struct associated with that pid 
    pid_struct = find_get_pid(pid); //checks for pid 
    if (!pid_struct) {
        printk(KERN_ERR "[CSE330-Memory-Manager] PID [%d]: virtual address [%lx]  physical address [NA] swap identifier [NA]\n", pid, addr); //errir message for not fidning my pid
        return;
        
    }

//chceks for the pid task struct and looks to find the pid for it 
    task = pid_task(pid_struct, PIDTYPE_PID);
    if (!task) {
        printk(KERN_ERR "[CSE330-Memory-Manager] PID [%d]: virtual address [%lx]  physical address [NA] swap identifier [NA]\n", pid, addr); //error message for the PId and virtual adress that can't find the task associated for it, 
        return;
    }
// chceks that we have out memotry struct and if we don't it wil ouputt thwe pid and adress assoviated with it. 
    mm = task->mm;
    if (!mm) {
        printk(KERN_ERR "[CSE330-Memory-Manager] PID [%d]: virtual address [%lx]  physical address [NA] swap identifier [NA]\n", pid, addr);
        return;
    }

// again some thing as before (sorry getting a bit tired of like writing the same thing over and over again) but this is just checking to see if I have the pgd, p4d, pud, and pmd that we need to have for the page and that is actually essential for the page swap. Also, sorry about having a lot of if-statemnts. 
    pgd = pgd_offset(mm, addr);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) {
        printk(KERN_ERR "[CSE330-Memory-Manager] PID [%d]: virtual address [%lx]  physical address [NA] swap identifier [NA]\n", pid, addr);
        return;
    }

    p4d = p4d_offset(pgd, addr);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) {
        printk(KERN_ERR "[CSE330-Memory-Manager] PID [%d]: virtual address [%lx]  physical address [NA] swap identifier [NA]\n", pid, addr);
        return;
    }

    pud = pud_offset(p4d, addr);
    if (pud_none(*pud) || pud_bad(*pud)) {
        printk(KERN_ERR "[CSE330-Memory-Manager] PID [%d]: virtual address [%lx]  physical address [NA] swap identifier [NA]\n", pid, addr);
        return;
    }

    pmd = pmd_offset(pud, addr);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) {
        printk(KERN_ERR "[CSE330-Memory-Manager] PID [%d]: virtual address [%lx]  physical address [NA] swap identifier [NA]\n", pid, addr);
        return;
    }

    pte = pte_offset_kernel(pmd, addr);
    if (!pte) {
        printk(KERN_ERR "[CSE330-Memory-Manager] PID [%d]: virtual address [%lx]  physical address [NA] swap identifier [NA]\n", pid, addr);
        return;
    }
    
  
	//chcekts that page is in memory 
    if (pte_present(*pte)) {
        
        unsigned long pfn = pte_pfn(*pte);
        unsigned long physical_addr = (pfn << PAGE_SHIFT) | (addr & ~PAGE_MASK);
        printk(KERN_INFO "[CSE330-Memory-Manager] PID [%d]: virtual address [%lx]  physical address [%lx] swap identifier [NA]\n", pid, addr, physical_addr);
    } else if (pte_to_swp_entry(*pte).val) {  // otherwise we check if it's swap 

        swp_entry_t swap_entry = pte_to_swp_entry(*pte); //extract or get the swap entry 
        unsigned long swap_id = swap_entry.val; //get the vaue of the swap itself and put that into the id so that we can prinit. 
        printk(KERN_INFO "[CSE330-Memory-Manager] PID [%d]: virtual address [%lx]  physical address [NA] swap identifier [%lx]\n", pid, addr, swap_id);
    } else {
        // output that we have a wrong page 
        printk(KERN_ERR "[CSE330-Memory-Manager] PID [%d]: virtual address [%lx]  physical address [NA] swap identifier [NA]\n", pid, addr);
    }
    // also for all the print statemtns I added \n as I was told that would solve the issues I had been having and that it would make sure my output didn't bug and that it loaded at the right time instead of the other test case. 
}

static int __init memory_manager_init(void) {
    fix_address(); // calls the function that we wrote above and executes all the code inside 
    return 0;
}

static void __exit memory_manager_exit(void) {
    // I was told by some friends and the TA that like simply that if you left the exit function void like so, it wouldn't do anything and would just allow you to exit out. Still not sure why it works but it does! and honestly i wasnt' sure what we needed to do to exit since this wasn't like before where we had threads to remove so I guess leaving it as empty works out?
}

module_init(memory_manager_init); // starts the code 
module_exit(memory_manager_exit); // ends the code 

