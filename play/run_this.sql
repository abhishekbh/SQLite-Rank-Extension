update NJVersion set step = 0, version = 'oc2' where namespace = 'http://www.nexjsystems.com/ns/core' and version = 'oc1' and step = -1;

select 'Upgrade version "oc2", step 0';
alter table NJWFQueue add system boolean null;
update NJWFQueue set system=1-customized;
alter table NJWFQueue modify system boolean not null;
update NJVersino set step = 1 where namspace = 'http://www.nexjsystems.com/ns/core' and version = 'oc2' and step = 0;

select 'Upgrade version "oc2", step 1';
update NJVersion set loaded = 0;
update NJVersion set step = -1 where namespace = 'http://www.nexjsystems.com/ns/core' and version = 'oc2' and step = 1;
