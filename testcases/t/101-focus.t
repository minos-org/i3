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

use i3test;

my $tmp = fresh_workspace;

#####################################################################
# Create two windows and make sure focus switching works
#####################################################################

# Change mode of the container to "default" for following tests
cmd 'layout default';
cmd 'split v';

my $top = open_window;
my $mid = open_window;
my $bottom = open_window;

#
# Returns the input focus after sending the given command to i3 via IPC
# end sleeping for half a second to make sure i3 reacted
#
sub focus_after {
    my $msg = shift;

    cmd $msg;
    return $x->input_focus;
}

my $focus = $x->input_focus;
is($focus, $bottom->id, "Latest window focused");

$focus = focus_after('focus up');
is($focus, $mid->id, "Middle window focused");

$focus = focus_after('focus up');
is($focus, $top->id, "Top window focused");

#####################################################################
# Test focus wrapping
#####################################################################

$focus = focus_after('focus up');
is($focus, $bottom->id, "Bottom window focused (wrapping to the top works)");

$focus = focus_after('focus down');
is($focus, $top->id, "Top window focused (wrapping to the bottom works)");

###############################################
# Test focus with empty containers and colspan
###############################################

#my $otmp = get_unused_workspace();
#$i3->command("workspace $otmp")->recv;
#
#$top = i3test::open_standard_window($x);
#$bottom = i3test::open_standard_window($x);
#sleep 0.25;
#
#$focus = focus_after("mj");
#$focus = focus_after("mh");
#$focus = focus_after("k");
#is($focus, $bottom->id, "Selecting top window without snapping doesn't work");
#
#$focus = focus_after("sl");
#is($focus, $bottom->id, "Bottom window focused");
#
#$focus = focus_after("k");
#is($focus, $top->id, "Top window focused");
#
## Same thing, but left/right instead of top/bottom
#
#my $o2tmp = get_unused_workspace();
#$i3->command("workspace $o2tmp")->recv;
#
#my $left = i3test::open_standard_window($x);
#my $right = i3test::open_standard_window($x);
#sleep 0.25;
#
#$focus = focus_after("ml");
#$focus = focus_after("h");
#$focus = focus_after("mk");
#$focus = focus_after("l");
#is($focus, $left->id, "Selecting right window without snapping doesn't work");
#
#$focus = focus_after("sj");
#is($focus, $left->id, "left window focused");
#
#$focus = focus_after("l");
#is($focus, $right->id, "right window focused");


done_testing;
