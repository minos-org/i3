#!perl
# vim:ts=4:sw=4:expandtab
#
# Please read the following documents before working on tests:
# • http://build.i3wm.org/docs/testsuite.html
#   (or docs/testsuite)
#
# • http://build.i3wm.org/docs/lib-i3test.html
#   (alternatively: perldoc ./testcases/lib/i3test.pm)
#
# • http://build.i3wm.org/docs/ipc.html
#   (or docs/ipc)
#
# • http://onyxneon.com/books/modern_perl/modern_perl_a4.pdf
#   (unless you are already familiar with Perl)
#
# Tests whether i3 crashes on cross-output moves with one workspace per output.
# Ticket: #827
# Bug still in: 4.3-78-g66b389c
#
use List::Util qw(first);
use i3test i3_autostart => 0;

# Ensure the pointer is at (0, 0) so that we really start on the first
# (the left) workspace.
$x->root->warp_pointer(0, 0);

my $config = <<EOT;
# i3 config file (v4)
font -misc-fixed-medium-r-normal--13-120-75-75-C-70-iso10646-1

fake-outputs 1024x768+0+0,1024x768+1024+0
EOT
my $pid = launch_with_config($config);

################################################################################
# Setup workspaces so that they stay open (with an empty container).
################################################################################

is(focused_ws, '1', 'starting on workspace 1');

cmd 'move workspace to output fake-1';

does_i3_live;

exit_gracefully($pid);

done_testing;
