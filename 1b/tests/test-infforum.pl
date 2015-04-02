#!/usr/bin/perl

use strict;
use warnings;

use Statistics::Histogram;
use Time::HiRes qw( sleep );

my @results;
my $alwaysErrorContinue = 0;

for(my $num = sprintf("%d", 000000); $num <= sprintf("%d", 077777); $num++){
        my $combo = sprintf("%05o", $num);    
            
        $combo =~ s/0/b/g;
        $combo =~ s/1/d/g;
        $combo =~ s/2/g/g;
        $combo =~ s/3/o/g;
        $combo =~ s/4/r/g;
        $combo =~ s/5/s/g;
        $combo =~ s/6/v/g;
        $combo =~ s/7/w/g;
        
        system("$ARGV[0] $ARGV[2] $combo > /dev/null &");
        sleep(0.05);
        my $result =`$ARGV[1] localhost $ARGV[2]`;
        chomp($result);

        if($result !~ m/\d+/){
            if($alwaysErrorContinue != 1){
                my $errorContinue = -1;
                print("Client lost the game for combination: $combo.\n");
                print("This will get you NO bonus points regardless of the mean tries. Do you want to continue anyway? [y|n]\n");
            
                while($errorContinue != 0 && $errorContinue != 1){
                    my $answer = <STDIN>;
                    chomp($answer);
                    
                    if($answer eq 'y'){
                        $errorContinue = 1;
                    }
                    elsif($answer eq 'n'){
                        $errorContinue = 0;
                    }
                }

                if($errorContinue == 1){
                    print("Do you want to always continue for all failed games that might occure?\n");

                    my $alwaysErrorContinueTemp = -1;    
                    while($alwaysErrorContinueTemp != 0 && $alwaysErrorContinueTemp != 1){
                        my $alwaysAnswer = <STDIN>;
                        chomp($alwaysAnswer);
                        
                        if($alwaysAnswer eq 'y'){
                            $alwaysErrorContinueTemp = 1;
                            $alwaysErrorContinue = 1;
                        }
                        elsif($alwaysAnswer eq 'n'){
                            $alwaysErrorContinueTemp = 0;
                        }
                    }    
                    
                    next;
                    
                }
                else{
                    last;
                }
            }
            else{
                next;
            }
        }
        else{    
            push(@results, $result);

            system("killall $ARGV[0] 2> /dev/null");
            system("killall $ARGV[1] 2> /dev/null");



            if(scalar(@results) > 1){


                my %counts;
                $counts{$_}++ for @results;
                my @unique = keys(%counts);

                print(get_histogram(\@results, scalar(@unique), 1, 1));
            }
        }

}