################################################################################
# Verify that entries corresponding to the GCS performance schema instrumented
# threads are in the threads table only during the execution of the
# group replication plugin.
#
# Test:
# 0. The test requires one Server: M1
# 1. Verify that no entries corresponding to the instrumented GCS threads are
#    in the threads table.
# 2. Start GR on M1.
# 3. Verify that the entries corresponding to the instrumented GCS threads are
#    in the threads table.
# 4. Stop GR on M1.
# 5. Verify that no entries corresponding to the instrumented GCS threads
#    remain in the threads table.
# 6. Test end.
################################################################################

--source include/have_group_replication_plugin.inc
--let $rpl_skip_group_replication_start= 1
--source include/group_replication.inc

--echo
--echo ############################################################
--echo # 1. Verify that the entries corresponding to the
--echo #    instrumented GCS threads are not in the threads
--echo #    table immediately after plugin installation
--echo #    (before starting the GR).
--connection server1
--sorted_result
SELECT * FROM performance_schema.threads WHERE NAME LIKE '%THD_Gcs_%';
--let $assert_text= 'There should be no GCS instrumented threads entries in the threads table before starting the GR'
--let $assert_cond= COUNT(*) = 0 FROM performance_schema.threads WHERE NAME LIKE "%THD_Gcs_%"
--source include/assert.inc

--echo
--echo ############################################################
--echo # 2. Start the GR
--source include/start_and_bootstrap_group_replication.inc

let $have_xcom_stack= `SELECT @@group_replication_communication_stack LIKE 'XCOM'`;
--let $rpl_gcs_number_of_threads=
if ($have_xcom_stack)
{
  --let $rpl_gcs_number_of_threads= 4
}
if (!$have_xcom_stack)
{
  --let $rpl_gcs_number_of_threads= 4
}

--echo
--echo ############################################################
--echo # 3. Verify that the entries corresponding to the
--echo #    instrumented GCS threads are in the threads table
--echo #    after starting the GR.
--sorted_result
--let $assert_text= 'There should be $rpl_gcs_number_of_threads GCS instrumented thread entries in the threads table after starting the GR'
--let $assert_cond= COUNT(*) = $rpl_gcs_number_of_threads FROM performance_schema.threads WHERE NAME LIKE "%THD_Gcs_%"
--source include/assert.inc

--echo
--echo ############################################################
--echo # 4. Stop the GR
--source include/stop_group_replication.inc

--echo
--echo ############################################################
--echo # 5. Verify that no entries corresponding to the
--echo #    instrumented GCS threads remain in the threads table
--echo #    after stopping the GR.
--sorted_result
SELECT * FROM performance_schema.threads WHERE NAME LIKE '%THD_Gcs_%';
--let $assert_text= 'There should be no extra psi keys added after starting the GR'
--let $assert_cond= COUNT(*) = 0 FROM performance_schema.threads WHERE NAME LIKE "%THD_Gcs_%"
--source include/assert.inc

--echo
--echo ############################################################
--echo # 6. Test end.
--source include/group_replication_end.inc
