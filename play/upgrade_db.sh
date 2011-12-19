# An sql file exists with all the upgrade statements
# This file be executed by the framework once the sql file has been culled down to the necessary statements only
# This will then execute all those statements in a row
# sql file < run_this.sql

.bail ON
select case count(*)
    when 0 then 1/0
    else 1
    end
  from NJVersion
where version = 'oc2';

# another possible way to go about this:
# the script "interacts" with the .sqlite console by acting as a wrapper for it
