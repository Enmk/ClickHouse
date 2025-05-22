class AltinityWorkflowTemplates:
    JOB_SETUP_STEPS = """
      - name: Setup
        uses: ./.github/actions/runner_setup
"""
    ADDITIONAL_JOBS = r"""
  GrypeScan:
    needs: [config_workflow, docker_server_image, docker_keeper_image]
    if: ${{ !failure() && !cancelled() }}
    strategy:
      fail-fast: false
      matrix:
        include:
          - image: server
            suffix: ''
          - image: server
            suffix: '-alpine'
          - image: keeper
            suffix: ''
    uses: ./.github/workflows/grype_scan.yml
    secrets: inherit
    with:
      docker_image: altinityinfra/clickhouse-${{ matrix.image }}
      tag-suffix: ${{ matrix.suffix }}
#############################################################################################
##################################### REGRESSION TESTS ######################################
#############################################################################################
  RegressionTestsRelease:
    needs: [config_workflow, build_amd_release]
    if: ${{ !failure() && !cancelled() && !contains(fromJson(needs.config_workflow.outputs.data).ci_settings.exclude_keywords, 'regression')}}
    uses: ./.github/workflows/regression.yml
    secrets: inherit
    with:
      runner_type: altinity-on-demand, altinity-regression-tester
      commit: 7f798b66f2d2acf18cd202d9a0f39ec64fbc062b
      arch: release
      build_sha: ${{ github.event_name == 'pull_request' && github.event.pull_request.head.sha || github.sha }}
      timeout_minutes: 300
      workflow_config: ${{ needs.config_workflow.outputs.data }}
  RegressionTestsAarch64:
    needs: [config_workflow, build_arm_release]
    if: ${{ !failure() && !cancelled() && !contains(fromJson(needs.config_workflow.outputs.data).ci_settings.exclude_keywords, 'regression') && !contains(fromJson(needs.config_workflow.outputs.data).ci_settings.exclude_keywords, 'aarch64')}}
    uses: ./.github/workflows/regression.yml
    secrets: inherit
    with:
      runner_type: altinity-on-demand, altinity-regression-tester-aarch64
      commit: 7f798b66f2d2acf18cd202d9a0f39ec64fbc062b
      arch: aarch64
      build_sha: ${{ github.event_name == 'pull_request' && github.event.pull_request.head.sha || github.sha }}
      timeout_minutes: 300
      workflow_config: ${{ needs.config_workflow.outputs.data }}
  SignRelease:
    needs: [config_workflow, build_amd_release]
    if: ${{ !failure() && !cancelled() }}
    uses: ./.github/workflows/reusable_sign.yml
    secrets: inherit
    with:
      test_name: Sign release
      runner_type: altinity-style-checker
      data: ${{ needs.config_workflow.outputs.data }}
  SignAarch64:
    needs: [config_workflow, build_arm_release]
    if: ${{ !failure() && !cancelled() }}
    uses: ./.github/workflows/reusable_sign.yml
    secrets: inherit
    with:
      test_name: Sign aarch64
      runner_type: altinity-style-checker-aarch64
      data: ${{ needs.config_workflow.outputs.data }}
  FinishCIReport:
    if: ${{ !cancelled() }}
    needs:
      - config_workflow
      - dockers_build_amd_and_merge
      - build_amd_release
      - sqlancer_amd_debug
      - sqltest
      - SignRelease
      - SignAarch64
      - RegressionTestsRelease
      - RegressionTestsAarch64
      - GrypeScan
    runs-on: [self-hosted, altinity-on-demand, altinity-style-checker-aarch64]
    steps:
      - name: Check out repository code
        uses: Altinity/checkout@19599efdf36c4f3f30eb55d5bb388896faea69f6
        with:
          clear-repository: true
      - name: Finalize workflow report
        if: ${{ !cancelled() }}
        uses: ./.github/actions/create_workflow_report
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
          CHECKS_DATABASE_HOST: ${{ secrets.CHECKS_DATABASE_HOST }}
          CHECKS_DATABASE_USER: ${{ secrets.CLICKHOUSE_TEST_STAT_LOGIN }}
          CHECKS_DATABASE_PASSWORD: ${{ secrets.CLICKHOUSE_TEST_STAT_PASSWORD }}
        with:
          final: true
"""
