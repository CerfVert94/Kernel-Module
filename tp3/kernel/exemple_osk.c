/* osk.c */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <asm/uaccess.h>        /* copy_*_user */

#define         S_MAX   4                       /* taille du tampon circulaire  */
#define         OSK_MAJOR       124             /* */

static int      osk_open(struct inode *, struct file *);
static int      osk_release(struct inode *, struct file *);
static ssize_t  osk_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t  osk_write(struct file *, const char __user *, size_t, loff_t *);

static struct file_operations osk_fops = {                                       
       .read  = osk_read,
       .write = osk_write,                                                       
       .open  = osk_open,                                                        
       .release = osk_release                                                    
};          

static void init_tamp(void);
static int  ecr_tamp(char c);
static int  lec_tamp(char *p);
static int  vide(void);
static int  plein(void);

static DECLARE_WAIT_QUEUE_HEAD(p_wait_vide);
static DECLARE_WAIT_QUEUE_HEAD(p_wait_plein);

/*-----------------------------------------------------------------------------
   Ouverture du pilote
-----------------------------------------------------------------------------*/
static int osk_open(struct inode * inodep, struct file * filep)
{
        if(MINOR(inodep->i_rdev) != 0) {
                printk("osk_c : mauvais numero d'unite\n");
                return -EINVAL;
        }
        return 0;
}

/*-----------------------------------------------------------------------------
   Fermeture du pilote
-----------------------------------------------------------------------------*/
static int osk_release(struct inode * inodep, struct file * filep)
{
        return 0;
}
/*-----------------------------------------------------------------------------
        Read
-----------------------------------------------------------------------------*/
static ssize_t osk_read(struct file *filep, char __user *bufp, size_t count, loff_t *posp)
{
        char c;
        unsigned long cnt=0;

        if(vide() && count) {
                 printk("/dev/osk : read en sommeil %d\n",current->pid);
                wait_event_interruptible(p_wait_vide,!vide());
                 printk("/dev/osk : read reveil %d\n",current->pid);
        }

        while((count--) && (lec_tamp(&c) >= 0)) {
                put_user(c,bufp++);
                cnt++;
        }
        wake_up_interruptible(&p_wait_plein);
        return cnt;
    }
/*-----------------------------------------------------------------------------
        Write
-----------------------------------------------------------------------------*/
static ssize_t osk_write(struct file *filep, const char __user *bufp, size_t count, loff_t *posp)
{
        char c;
        unsigned long cnt=0;

        if(plein() && count) {
                 printk("/dev/osk : write en sommeil %d\n",current->pid);
                wait_event_interruptible(p_wait_plein,!plein());
                 printk("/dev/osk : write  reveil %d\n",current->pid);
        }

        while((count--) && !plein()) {
                get_user(c,bufp++);
                ecr_tamp(c);
                cnt++;
                }
        wake_up_interruptible(&p_wait_vide);
        return cnt;
}

/*-----------------------------------------------------------------------------
        Gestion du tampon circulaire
-----------------------------------------------------------------------------*/
static struct t_cir {
        char    tab[S_MAX];
        int             head;
        int             tail;
        int             full;   /* 1 : plein, 0 : non plein  */
        int             empty;  /* 1 : vide,  0 : non vide    */
        } t_cir ;

/*---------------------------------------------------------------------------*/
static void init_tamp(void)
{
        t_cir.head=0;
        t_cir.tail=0;
        t_cir.full=0;
        t_cir.empty=1;
        return ;
}

/*---------------------------------------------------------------------------*/
static int ecr_tamp(char c)
{
        if(t_cir.full) return -1;
        t_cir.tab[t_cir.head] = c;
        t_cir.empty = 0;
        t_cir.head++;
        if(t_cir.head >= S_MAX) t_cir.head = 0;
        if(t_cir.head == t_cir.tail)  t_cir.full = 1;
        return 0;
}

/*---------------------------------------------------------------------------*/
static int lec_tamp(char *p)
{
        if(t_cir.empty) return -1;
        *p = t_cir.tab[t_cir.tail];
        t_cir.full = 0;
        t_cir.tail++;
        if(t_cir.tail >= S_MAX) t_cir.tail = 0;
        if(t_cir.head == t_cir.tail)  t_cir.empty = 1;
        return 0;
}

/*---------------------------------------------------------------------------*/
static int vide(void)
{
        return t_cir.empty;
}

/*---------------------------------------------------------------------------*/
static int plein(void)
{
        return t_cir.full;
}

static int __init osk_init(void)
{
    int err = 0;

    if ((err = register_chrdev(OSK_MAJOR,"osk",&osk_fops)) != 0) 
    {
	pr_alert("*** unable to get major OSK_MAJOR for osk devices ***\n");
    }
    init_tamp();
    pr_alert("Good morning from osk, char device driver.\n");

    return 0;
}

static void __exit osk_exit(void)
{
    unregister_chrdev(OSK_MAJOR,"osk");
    pr_alert("Good bye from osk, char device driver.\n");
}

module_init(osk_init);
module_exit(osk_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Osk module");
MODULE_AUTHOR("F. Pecheux");
